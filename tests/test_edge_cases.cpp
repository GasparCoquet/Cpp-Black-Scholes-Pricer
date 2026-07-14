#include "../src/BlackScholes.h"
#include "check.h"

#include <cmath>
#include <limits>

using BS = BlackScholes;

// ---------------------------------------------------------------------------
// The degenerate inputs that used to return a silent NaN.
//
// These are not exotic. T = 0 is every option on its expiry date. sigma = 0 is
// what a vol solver hands you when it hits its lower bound. A silent NaN from
// any of them propagates through a P&L or a risk aggregation and turns the
// whole number into NaN, with no indication of where it came from.
// ---------------------------------------------------------------------------

void test_expiry_T_equals_zero() {
    const double r = 0.05, sig = 0.2, q = 0.02;

    // At expiry an option is worth exactly its intrinsic value.
    CHECK_NEAR(BS::call_price(120, 100, r, 0.0, sig, q), 20.0, 0.0);
    CHECK_NEAR(BS::call_price(80, 100, r, 0.0, sig, q), 0.0, 0.0);
    CHECK_NEAR(BS::put_price(80, 100, r, 0.0, sig, q), 20.0, 0.0);
    CHECK_NEAR(BS::put_price(120, 100, r, 0.0, sig, q), 0.0, 0.0);

    // Delta is the indicator of finishing in the money.
    CHECK_NEAR(BS::call_delta(120, 100, r, 0.0, sig, q), 1.0, 0.0);
    CHECK_NEAR(BS::call_delta(80, 100, r, 0.0, sig, q), 0.0, 0.0);
    CHECK_NEAR(BS::put_delta(80, 100, r, 0.0, sig, q), -1.0, 0.0);
    CHECK_NEAR(BS::put_delta(120, 100, r, 0.0, sig, q), 0.0, 0.0);

    // Exactly at the money at expiry: the payoff is kinked, and the limit of
    // N(d1) as T -> 0 is N(0) = 0.5, not 0 or 1.
    CHECK_NEAR(BS::call_delta(100, 100, r, 0.0, sig, q), 0.5, 0.0);
    CHECK_NEAR(BS::put_delta(100, 100, r, 0.0, sig, q), -0.5, 0.0);

    // Gamma is a Dirac mass on the kink and zero off it. Vega and rho are zero:
    // no time left for either to act.
    CHECK_NEAR(BS::gamma(120, 100, r, 0.0, sig, q), 0.0, 0.0);
    CHECK(std::isinf(BS::gamma(100, 100, r, 0.0, sig, q)));
    CHECK(BS::gamma(100, 100, r, 0.0, sig, q) > 0);  // +inf, not -inf
    CHECK_NEAR(BS::vega(120, 100, r, 0.0, sig, q), 0.0, 0.0);
    CHECK_NEAR(BS::vega(100, 100, r, 0.0, sig, q), 0.0, 0.0);
    CHECK_NEAR(BS::call_rho(120, 100, r, 0.0, sig, q), 0.0, 0.0);
    CHECK_NEAR(BS::put_rho(80, 100, r, 0.0, sig, q), 0.0, 0.0);

    // Theta at expiry: an ITM call is pure carry (q*S - r*K); an OTM one has
    // nothing left to decay; an ATM one decays infinitely fast.
    CHECK_NEAR(BS::call_theta(120, 100, r, 0.0, sig, q), q * 120 - r * 100, 1e-12);
    CHECK_NEAR(BS::call_theta(80, 100, r, 0.0, sig, q), 0.0, 0.0);
    CHECK(std::isinf(BS::call_theta(100, 100, r, 0.0, sig, q)));
    CHECK(BS::call_theta(100, 100, r, 0.0, sig, q) < 0);  // -inf

    // Nothing here is a NaN. That is the whole point.
    CHECK(!std::isnan(BS::call_price(100, 100, r, 0.0, sig, q)));
    CHECK(!std::isnan(BS::vega(100, 100, r, 0.0, sig, q)));
}

void test_zero_volatility() {
    const double r = 0.05, T = 2.0, q = 0.0;

    // With no volatility the spot compounds to its forward with certainty, so
    // the option is worth its discounted intrinsic value.
    const double A = 120.0;                      // S*e^{-qT}, q = 0
    const double B = 100.0 * std::exp(-r * T);   // K*e^{-rT}
    CHECK_NEAR(BS::call_price(120, 100, r, T, 0.0, q), A - B, 1e-12);
    CHECK_NEAR(BS::put_price(120, 100, r, T, 0.0, q), 0.0, 0.0);

    // Below the forward, the call is worthless and the put is the certain payoff.
    const double A2 = 80.0;
    CHECK_NEAR(BS::call_price(80, 100, r, T, 0.0, q), 0.0, 0.0);
    CHECK_NEAR(BS::put_price(80, 100, r, T, 0.0, q), B - A2, 1e-12);

    // Deterministic payoff => delta is 0 or 1, gamma is 0.
    CHECK_NEAR(BS::call_delta(120, 100, r, T, 0.0, q), 1.0, 0.0);
    CHECK_NEAR(BS::call_delta(80, 100, r, T, 0.0, q), 0.0, 0.0);
    CHECK_NEAR(BS::gamma(120, 100, r, T, 0.0, q), 0.0, 0.0);
    CHECK_NEAR(BS::vega(120, 100, r, T, 0.0, q), 0.0, 0.0);

    // Zero-vol prices still satisfy put-call parity exactly.
    CHECK_NEAR(BS::call_price(120, 100, r, T, 0.0, q) - BS::put_price(120, 100, r, T, 0.0, q),
               120.0 - 100.0 * std::exp(-r * T), 1e-12);

    CHECK(!std::isnan(BS::call_price(100, 100, r, T, 0.0, q)));
    CHECK(!std::isnan(BS::gamma(100, 100, r, T, 0.0, q)));
}

void test_zero_spot() {
    const double K = 100, r = 0.05, T = 1.0, sig = 0.2, q = 0.0;

    // Zero is absorbing: the underlying is worthless and stays worthless. The
    // call expires worthless; the put is a certain payoff of K, worth K*e^{-rT}.
    CHECK_NEAR(BS::call_price(0.0, K, r, T, sig, q), 0.0, 0.0);
    CHECK_NEAR(BS::put_price(0.0, K, r, T, sig, q), K * std::exp(-r * T), 1e-12);

    CHECK_NEAR(BS::call_delta(0.0, K, r, T, sig, q), 0.0, 0.0);
    CHECK_NEAR(BS::put_delta(0.0, K, r, T, sig, q), -1.0, 1e-12);
    CHECK_NEAR(BS::gamma(0.0, K, r, T, sig, q), 0.0, 0.0);
    CHECK_NEAR(BS::vega(0.0, K, r, T, sig, q), 0.0, 0.0);

    // The put is a zero-coupon bond: its theta is its accretion, r*K*e^{-rT}.
    CHECK_NEAR(BS::put_theta(0.0, K, r, T, sig, q), r * K * std::exp(-r * T), 1e-12);
    CHECK_NEAR(BS::call_theta(0.0, K, r, T, sig, q), 0.0, 0.0);

    CHECK(!std::isnan(BS::gamma(0.0, K, r, T, sig, q)));
    CHECK(!std::isnan(BS::put_price(0.0, K, r, T, sig, q)));

    // Zero strike: the call IS the (dividend-adjusted) forward, the put is worthless.
    CHECK_NEAR(BS::call_price(100, 0.0, r, T, sig, 0.03), 100 * std::exp(-0.03 * T), 1e-12);
    CHECK_NEAR(BS::put_price(100, 0.0, r, T, sig, 0.03), 0.0, 0.0);
    CHECK_NEAR(BS::gamma(100, 0.0, r, T, sig, 0.03), 0.0, 0.0);
}

// ---------------------------------------------------------------------------
// Genuinely meaningless inputs. These are caller bugs, not limiting cases, and
// they get a loud NaN rather than a plausible-looking number.
// ---------------------------------------------------------------------------

void test_invalid_inputs_are_nan() {
    const double S = 100, K = 100, r = 0.05, T = 1.0, sig = 0.2;
    const double nan = std::numeric_limits<double>::quiet_NaN();
    const double inf = std::numeric_limits<double>::infinity();

    CHECK(std::isnan(BS::call_price(-1.0, K, r, T, sig)));   // negative spot
    CHECK(std::isnan(BS::call_price(S, -1.0, r, T, sig)));   // negative strike
    CHECK(std::isnan(BS::call_price(S, K, r, -1.0, sig)));   // negative time
    CHECK(std::isnan(BS::call_price(S, K, r, T, -0.2)));     // negative vol
    CHECK(std::isnan(BS::put_price(S, K, r, T, -0.2)));
    CHECK(std::isnan(BS::gamma(S, K, r, T, -0.2)));
    CHECK(std::isnan(BS::call_price(nan, K, r, T, sig)));    // NaN in
    CHECK(std::isnan(BS::call_price(inf, K, r, T, sig)));    // inf in
    CHECK(std::isnan(BS::call_price(S, K, nan, T, sig)));
    CHECK(std::isnan(BS::call_price(S, K, r, T, sig, nan))); // NaN dividend
}

int main() {
    test_expiry_T_equals_zero();
    test_zero_volatility();
    test_zero_spot();
    test_invalid_inputs_are_nan();
    return test::report("Edge cases");
}
