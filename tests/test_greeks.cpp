#include "../src/BlackScholes.h"
#include "check.h"

#include <cmath>
#include <cstdio>
#include <vector>

// NOTE: these assertions used to be bare assert().  CI builds Release, which
// defines NDEBUG, which turns assert() into a no-op -- so none of them ran.
// See check.h.  CHECK/CHECK_NEAR cannot be compiled away.

using BS = BlackScholes;

// ---------------------------------------------------------------------------
// Reference values
// ---------------------------------------------------------------------------

void test_reference_values() {
    // S=100 K=100 r=5% T=1y sigma=20%, no dividend.  10.4506 / 5.5735 are the
    // textbook figures every Black-Scholes calculator agrees on; the extra
    // digits are this implementation's, pinned so a regression cannot drift.
    const double S = 100, K = 100, r = 0.05, T = 1.0, sig = 0.2;
    CHECK_NEAR(BS::call_price(S, K, r, T, sig), 10.4506, 1e-4);
    CHECK_NEAR(BS::put_price(S, K, r, T, sig), 5.5735, 1e-4);

    CHECK_NEAR(BS::call_price(S, K, r, T, sig), 10.450583572185565, 1e-12);
    CHECK_NEAR(BS::put_price(S, K, r, T, sig), 5.5735260222569707, 1e-12);
    CHECK_NEAR(BS::call_delta(S, K, r, T, sig), 0.6368306511756191, 1e-12);
    CHECK_NEAR(BS::put_delta(S, K, r, T, sig), -0.3631693488243809, 1e-12);
    CHECK_NEAR(BS::gamma(S, K, r, T, sig), 0.018762017345846895, 1e-12);
    CHECK_NEAR(BS::vega(S, K, r, T, sig), 37.524034691693792, 1e-10);
    CHECK_NEAR(BS::call_theta(S, K, r, T, sig), -6.4140275464381968, 1e-10);
    CHECK_NEAR(BS::put_theta(S, K, r, T, sig), -1.6578804239346265, 1e-10);
    CHECK_NEAR(BS::call_rho(S, K, r, T, sig), 53.232481545376345, 1e-10);
    CHECK_NEAR(BS::put_rho(S, K, r, T, sig), -41.890460904695061, 1e-10);
}

// ---------------------------------------------------------------------------
// Signs and monotonicity: the cheap sanity net
// ---------------------------------------------------------------------------

void test_signs_and_bounds() {
    const double S = 100, K = 100, r = 0.05, T = 1.0, sig = 0.2;
    CHECK(BS::call_delta(S, K, r, T, sig) > 0.6 && BS::call_delta(S, K, r, T, sig) < 0.7);
    CHECK(BS::put_delta(S, K, r, T, sig) > -0.4 && BS::put_delta(S, K, r, T, sig) < -0.3);
    CHECK(BS::gamma(S, K, r, T, sig) > 0);       // long an option = long gamma
    CHECK(BS::vega(S, K, r, T, sig) > 0);        // ...and long vega
    CHECK(BS::call_theta(S, K, r, T, sig) < 0);  // ...and short theta
    CHECK(std::isfinite(BS::put_theta(S, K, r, T, sig)));
    CHECK(BS::call_rho(S, K, r, T, sig) > 0);
    CHECK(BS::put_rho(S, K, r, T, sig) < 0);

    // Deep ITM call delta -> 1, deep OTM -> 0.
    CHECK(BS::call_delta(150, 100, 0.05, 1.0, 0.2) > 0.99);
    CHECK(BS::call_delta(50, 100, 0.05, 1.0, 0.2) < 0.01);

    // Gamma rises as expiry approaches.
    CHECK(BS::gamma(100, 100, 0.05, 0.01, 0.2) > BS::gamma(100, 100, 0.05, 1.0, 0.2));

    // Prices respect the no-arbitrage bounds.
    CHECK(BS::call_price(S, K, r, T, sig) >= std::fmax(S - K * std::exp(-r * T), 0.0));
    CHECK(BS::call_price(S, K, r, T, sig) < S);
    CHECK(BS::put_price(S, K, r, T, sig) >= std::fmax(K * std::exp(-r * T) - S, 0.0));
    CHECK(BS::put_price(S, K, r, T, sig) < K * std::exp(-r * T));
}

// ---------------------------------------------------------------------------
// Exact identities.  These must hold to floating-point precision, not
// "approximately": they are static replication arguments, not approximations.
// ---------------------------------------------------------------------------

struct Params {
    double S, K, r, T, sigma, q;
};

static std::vector<Params> grid() {
    std::vector<Params> g;
    for (double S : {60.0, 80.0, 100.0, 120.0, 150.0})
        for (double K : {70.0, 100.0, 130.0})
            for (double r : {0.0, 0.03, 0.08})
                for (double T : {0.05, 0.5, 1.0, 3.0})
                    for (double sig : {0.05, 0.2, 0.6, 1.2})
                        for (double q : {0.0, 0.02, 0.06}) g.push_back({S, K, r, T, sig, q});
    return g;
}

void test_put_call_parity() {
    // C - P = S*e^{-qT} - K*e^{-rT}.  Note the CONTINUOUS discount factor: an
    // earlier version of the Python example used 1/(1+r)^T here (annual
    // discrete compounding) and was off by 0.115 on the base case.
    for (const Params& p : grid()) {
        const double lhs = BS::call_price(p.S, p.K, p.r, p.T, p.sigma, p.q) -
                           BS::put_price(p.S, p.K, p.r, p.T, p.sigma, p.q);
        const double rhs = p.S * std::exp(-p.q * p.T) - p.K * std::exp(-p.r * p.T);
        CHECK_NEAR(lhs, rhs, 1e-10);
    }
}

void test_delta_parity() {
    // dC/dS - dP/dS = e^{-qT}  (it is 1 only when q = 0).
    for (const Params& p : grid()) {
        const double d = BS::call_delta(p.S, p.K, p.r, p.T, p.sigma, p.q) -
                         BS::put_delta(p.S, p.K, p.r, p.T, p.sigma, p.q);
        CHECK_NEAR(d, std::exp(-p.q * p.T), 1e-12);
    }
}

void test_dividend_is_a_spot_haircut() {
    // Pricing with a dividend yield q is identical to pricing a q=0 option on a
    // spot of S*e^{-qT}.  This is an exact algebraic identity, so if the q terms
    // were dropped into d1 wrongly (a classic sign slip) this fails immediately.
    for (const Params& p : grid()) {
        const double S_adj = p.S * std::exp(-p.q * p.T);
        CHECK_NEAR(BS::call_price(p.S, p.K, p.r, p.T, p.sigma, p.q),
                   BS::call_price(S_adj, p.K, p.r, p.T, p.sigma, 0.0), 1e-12);
        CHECK_NEAR(BS::put_price(p.S, p.K, p.r, p.T, p.sigma, p.q),
                   BS::put_price(S_adj, p.K, p.r, p.T, p.sigma, 0.0), 1e-12);
    }
}

// ---------------------------------------------------------------------------
// Every Greek against a central finite difference of the price.
//
// This is the test that actually pins the Greeks.  Asserting "gamma > 0" cannot
// tell a correct gamma from one that is 10x too large; differentiating the
// price numerically can.  It also catches the classic errors that sign checks
// sail past: a missing e^{-qT} on the spot leg, a factor of 2 in the theta
// diffusion term, or theta returned as dV/dT instead of dV/dt.
//
// The bump in S is scaled to the width of the terminal distribution,
// S*sigma*sqrt(T), rather than being a fixed fraction of S.  A fixed 1e-4*S bump
// is far too coarse for a 3-week 5%-vol option, whose whole distribution is
// narrower than the bump, and truncation error then swamps the comparison.
//
// Measured worst relative disagreement over this grid: 3.1e-07 (well inside the
// 1e-5 tolerance below, which leaves room for platform FP differences).
// ---------------------------------------------------------------------------

static constexpr double kFdTol = 1e-5;

void test_greeks_against_finite_differences() {
    for (const Params& p : grid()) {
        const double S = p.S, K = p.K, r = p.r, T = p.T, v = p.sigma, q = p.q;

        const double hS = 1e-3 * S * v * std::sqrt(T);  // scaled to the diffusion
        const double hv = 1e-5;
        const double hT = 1e-5;
        const double hr = 1e-6;

        const double c0 = BS::call_price(S, K, r, T, v, q);
        const double cUp = BS::call_price(S + hS, K, r, T, v, q);
        const double cDn = BS::call_price(S - hS, K, r, T, v, q);
        const double pUp = BS::put_price(S + hS, K, r, T, v, q);
        const double pDn = BS::put_price(S - hS, K, r, T, v, q);

        // Delta = dV/dS
        CHECK_CLOSE_REL(BS::call_delta(S, K, r, T, v, q), (cUp - cDn) / (2 * hS), kFdTol);
        CHECK_CLOSE_REL(BS::put_delta(S, K, r, T, v, q), (pUp - pDn) / (2 * hS), kFdTol);

        // Gamma = d2V/dS2
        CHECK_CLOSE_REL(BS::gamma(S, K, r, T, v, q), (cUp - 2 * c0 + cDn) / (hS * hS), kFdTol);

        // Vega = dV/dsigma
        CHECK_CLOSE_REL(BS::vega(S, K, r, T, v, q),
                        (BS::call_price(S, K, r, T, v + hv, q) -
                         BS::call_price(S, K, r, T, v - hv, q)) / (2 * hv),
                        kFdTol);

        // Theta = dV/dt = -dV/dT.  Note the minus sign: getting this backwards
        // is the single most common Greek bug and no sign-only test finds it.
        CHECK_CLOSE_REL(BS::call_theta(S, K, r, T, v, q),
                        -(BS::call_price(S, K, r, T + hT, v, q) -
                          BS::call_price(S, K, r, T - hT, v, q)) / (2 * hT),
                        kFdTol);
        CHECK_CLOSE_REL(BS::put_theta(S, K, r, T, v, q),
                        -(BS::put_price(S, K, r, T + hT, v, q) -
                          BS::put_price(S, K, r, T - hT, v, q)) / (2 * hT),
                        kFdTol);

        // Rho = dV/dr
        CHECK_CLOSE_REL(BS::call_rho(S, K, r, T, v, q),
                        (BS::call_price(S, K, r + hr, T, v, q) -
                         BS::call_price(S, K, r - hr, T, v, q)) / (2 * hr),
                        kFdTol);
        CHECK_CLOSE_REL(BS::put_rho(S, K, r, T, v, q),
                        (BS::put_price(S, K, r + hr, T, v, q) -
                         BS::put_price(S, K, r - hr, T, v, q)) / (2 * hr),
                        kFdTol);
    }
}

int main() {
    test_reference_values();
    test_signs_and_bounds();
    test_put_call_parity();
    test_delta_parity();
    test_dividend_is_a_spot_haircut();
    test_greeks_against_finite_differences();

    std::printf("finite-difference grid: %zu parameter sets x 8 Greeks\n", grid().size());
    return test::report("Black-Scholes Greeks");
}
