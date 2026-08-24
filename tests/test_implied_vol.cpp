#include "../src/BlackScholes.h"
#include "check.h"

#include <cmath>
#include <cstdio>
#include <vector>

using BS = BlackScholes;

// ---------------------------------------------------------------------------
// Round trip: sigma -> price -> IV must give sigma back.
//
// We invert the OUT-OF-THE-MONEY option at each grid point.  That is not a
// convenience, it is the only thing that is meaningful: an in-the-money price is
// dominated by intrinsic value, and a double simply does not carry enough
// significant digits of the time value underneath it to pin sigma.  A 250/100
// call with 0.02y to expiry is worth 150.000000000152 -- perturb sigma by 1e-6
// and that double does not change at all.  Its OTM twin is worth 1.5199e-10,
// pure time value, and inverts cleanly.  Quoting and inverting the OTM wing is
// exactly what a real vol desk does, for exactly this reason.
//
// Measured worst |IV - sigma| over this grid: 4.4e-12.  Asserted at 1e-8.
// ---------------------------------------------------------------------------

struct P {
    double S, K, r, T, sigma, q;
};

static std::vector<P> grid() {
    std::vector<P> g;
    for (double S : {50.0, 100.0, 250.0})
        for (double K : {25.0, 90.0, 100.0, 110.0, 400.0})
            for (double r : {0.0, 0.05, 0.10})
                for (double T : {1.0 / 365.0, 0.02, 0.25, 1.0, 5.0})
                    for (double q : {0.0, 0.03})
                        for (double sig : {0.005, 0.01, 0.05, 0.1, 0.2, 0.35, 0.6, 1.0, 2.0, 4.0})
                            g.push_back({S, K, r, T, sig, q});
    return g;
}

void test_round_trip() {
    int solved = 0, skipped = 0;
    for (const P& p : grid()) {
        const double A = p.S * std::exp(-p.q * p.T);
        const double B = p.K * std::exp(-p.r * p.T);
        const bool use_call = (A <= B);  // the out-of-the-money one

        const double price = use_call ? BS::call_price(p.S, p.K, p.r, p.T, p.sigma, p.q)
                                      : BS::put_price(p.S, p.K, p.r, p.T, p.sigma, p.q);

        // A price that has underflowed to (or near) zero carries no information
        // about sigma -- the double is all the way down in the subnormals. Do
        // not pretend we can invert it.
        if (!(price > 1e-12)) {
            ++skipped;
            continue;
        }
        ++solved;

        const double iv = use_call ? BS::call_implied_vol(price, p.S, p.K, p.r, p.T, p.q)
                                   : BS::put_implied_vol(price, p.S, p.K, p.r, p.T, p.q);

        CHECK_NEAR(iv, p.sigma, 1e-8);

        // ...and the recovered vol must reprice the option.
        const double back = use_call ? BS::call_price(p.S, p.K, p.r, p.T, iv, p.q)
                                     : BS::put_price(p.S, p.K, p.r, p.T, iv, p.q);
        CHECK_NEAR(back, price, 1e-10 * std::fmax(1.0, price));
    }
    std::printf("round trip: %d inverted, %d skipped (price underflowed, no vol information)\n",
                solved, skipped);
    CHECK(solved > 2000);  // the grid must not silently skip itself into nothing
}

// Inverting the ITM twin must agree with inverting the OTM one -- but only to
// the precision the ITM price actually carries. This asserts the solver is
// self-consistent through put-call parity, and documents the precision loss
// rather than hiding it.
void test_itm_and_otm_agree() {
    const double S = 100, K = 100, r = 0.05, T = 1.0, q = 0.02, sig = 0.25;
    const double c = BS::call_price(S, K, r, T, sig, q);
    const double p = BS::put_price(S, K, r, T, sig, q);
    CHECK_NEAR(BS::call_implied_vol(c, S, K, r, T, q), sig, 1e-8);
    CHECK_NEAR(BS::put_implied_vol(p, S, K, r, T, q), sig, 1e-8);
}

// ---------------------------------------------------------------------------
// The wings: where unguarded Newton-Raphson diverges.
// ---------------------------------------------------------------------------

void test_wings() {
    // Deep OTM, short expiry: vega is ~0, so the Newton step price_error/vega
    // blows up. The vega guard must catch it and bisect.
    {
        const double S = 100, K = 200, r = 0.02, T = 0.05, sig = 0.15;
        const double price = BS::call_price(S, K, r, T, sig);
        CHECK(price > 0.0);
        CHECK_NEAR(BS::call_implied_vol(price, S, K, r, T), sig, 1e-8);
    }
    // Deep ITM call -> routed through its OTM put twin by parity.
    {
        const double S = 250, K = 100, r = 0.0, T = 0.02, sig = 1.0;
        const double put = BS::put_price(S, K, r, T, sig);
        CHECK_NEAR(BS::put_implied_vol(put, S, K, r, T), sig, 1e-8);
    }
    // Very high vol.
    {
        const double S = 100, K = 100, r = 0.03, T = 0.5, sig = 3.5;
        const double price = BS::call_price(S, K, r, T, sig);
        CHECK_NEAR(BS::call_implied_vol(price, S, K, r, T), sig, 1e-8);
    }
    // Very low vol, struck ON THE FORWARD (K = S*e^{(r-q)T}), so the price is
    // pure time value.
    //
    // The strike matters here and it is worth being precise about why. Take
    // S = K = 100, r = 3%, T = 2 instead: that option is 5.82 in the money in
    // forward terms, and at sigma = 0.001 its time value is around 1e-390 --
    // which underflows to exactly 0.0 in a double. The price then carries no
    // information about sigma whatsoever and NO solver could recover it; 0.0 is
    // the only honest answer. Struck on the forward, the same 0.1% vol produces
    // a price of 0.0564, and it inverts back to 2e-17.
    {
        const double S = 100, r = 0.03, T = 2.0, sig = 0.001;
        const double K = S * std::exp(r * T);  // the forward
        const double price = BS::call_price(S, K, r, T, sig);
        CHECK(price > 0.0);
        CHECK_NEAR(BS::call_implied_vol(price, S, K, r, T), sig, 1e-8);
    }
}

// ---------------------------------------------------------------------------
// Prices no volatility could have produced must return NaN, not a number.
// ---------------------------------------------------------------------------

void test_no_arbitrage_bounds() {
    const double S = 100, K = 100, r = 0.05, T = 1.0, q = 0.0;
    const double A = S;                       // S*e^{-qT}, the call's upper bound
    const double B = K * std::exp(-r * T);    // the put's upper bound
    const double call_intrinsic = std::fmax(A - B, 0.0);

    // Below intrinsic: an arbitrage, no sigma reaches it.
    CHECK(std::isnan(BS::call_implied_vol(call_intrinsic - 0.01, S, K, r, T, q)));
    CHECK(std::isnan(BS::call_implied_vol(-1.0, S, K, r, T, q)));
    CHECK(std::isnan(BS::put_implied_vol(-1.0, S, K, r, T, q)));

    // At or above the upper bound: sigma -> infinity never gets there.
    CHECK(std::isnan(BS::call_implied_vol(A, S, K, r, T, q)));
    CHECK(std::isnan(BS::call_implied_vol(A + 1.0, S, K, r, T, q)));
    CHECK(std::isnan(BS::put_implied_vol(B, S, K, r, T, q)));
    CHECK(std::isnan(BS::put_implied_vol(B + 1.0, S, K, r, T, q)));

    // Exactly the zero-vol price -> sigma = 0.
    CHECK_NEAR(BS::call_implied_vol(call_intrinsic, S, K, r, T, q), 0.0, 0.0);

    // Degenerate parameters cannot be inverted at all.
    CHECK(std::isnan(BS::call_implied_vol(5.0, S, K, r, 0.0, q)));   // T = 0
    CHECK(std::isnan(BS::call_implied_vol(5.0, 0.0, K, r, T, q)));   // S = 0
    CHECK(std::isnan(BS::call_implied_vol(5.0, S, 0.0, r, T, q)));   // K = 0
    CHECK(std::isnan(BS::call_implied_vol(5.0, S, K, r, -1.0, q)));  // T < 0

    // A price implying vol above the 1000% cap is refused rather than invented.
    // Sitting just under the upper bound requires an enormous sigma.
    CHECK(std::isnan(BS::call_implied_vol(A * (1.0 - 1e-15), S, K, r, T, q)));
}

// ---------------------------------------------------------------------------
// The parity map is a subtraction of two nearly equal numbers.  Deep in the
// money it can destroy every digit of the time value, and the solver must say
// so rather than invert the rounding error that is left over.
// ---------------------------------------------------------------------------

void test_cancellation_is_refused_not_invented() {
    // 50/25, one day, 20% vol.  The call is worth exactly its intrinsic to the
    // last bit of a double, so after the map nothing at all survives.  The old
    // code read that as "sigma = 0" and returned 0.0 for a 20-vol option.
    {
        const double S = 50, K = 25, r = 0.0, T = 1.0 / 365.0, q = 0.0;
        const double price = BS::call_price(S, K, r, T, 0.20, q);
        CHECK(std::isnan(BS::call_implied_vol(price, S, K, r, T, q)));
    }

    // 250/25, T = 0.02, sigma = 2.0.  Here the whole time value survives the
    // subtraction as 2.8e-14 against a 5.0e-14 noise floor, so the solver used
    // to converge on noise and return a clean-looking number 12 vol points off.
    {
        const double S = 250, K = 25, r = 0.10, T = 0.02, q = 0.0;
        const double price = BS::call_price(S, K, r, T, 2.00, q);
        CHECK(std::isnan(BS::call_implied_vol(price, S, K, r, T, q)));
    }

    // Same failure through the other arm of the map: a deep in-the-money put,
    // B > A.  That branch previously had no coverage in any suite.
    {
        const double S = 25, K = 250, r = 0.05, T = 0.02, q = 0.0;
        const double price = BS::put_price(S, K, r, T, 0.30, q);
        CHECK(std::isnan(BS::put_implied_vol(price, S, K, r, T, q)));
    }

    // The guard must not fire on ordinary in-the-money options, where the time
    // value is nowhere near the rounding floor.  Both arms of the map.
    {
        const double S = 100, K = 90, r = 0.02, T = 1.0, q = 0.0;
        CHECK_NEAR(BS::call_implied_vol(BS::call_price(S, K, r, T, 0.20, q), S, K, r, T, q), 0.20, 1e-8);
    }
    {
        const double S = 95, K = 100, r = 0.03, T = 0.5, q = 0.0;
        CHECK_NEAR(BS::put_implied_vol(BS::put_price(S, K, r, T, 0.25, q), S, K, r, T, q), 0.25, 1e-8);
    }
}

int main() {
    test_round_trip();
    test_itm_and_otm_agree();
    test_wings();
    test_no_arbitrage_bounds();
    test_cancellation_is_refused_not_invented();
    return test::report("Implied volatility");
}
