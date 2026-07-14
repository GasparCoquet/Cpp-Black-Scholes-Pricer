#define _USE_MATH_DEFINES
#include "BlackScholes.h"

#include <algorithm>
#include <cmath>
#include <limits>

namespace {

constexpr double kNaN = std::numeric_limits<double>::quiet_NaN();
constexpr double kInf = std::numeric_limits<double>::infinity();

// Inputs that do not describe an option at all.  A negative spot, a negative
// strike, a negative time to expiry or a negative volatility are caller bugs,
// not limiting cases, so they get a loud NaN rather than a plausible number.
bool invalid(double S, double K, double r, double T, double sigma, double q) noexcept {
    return !std::isfinite(S) || !std::isfinite(K) || !std::isfinite(r) ||
           !std::isfinite(T) || !std::isfinite(sigma) || !std::isfinite(q) ||
           S < 0.0 || K < 0.0 || T < 0.0 || sigma < 0.0;
}

// Everything the closed form needs, computed once.
//
// A = S*e^{-qT} is the present value of receiving one share at expiry;
// B = K*e^{-rT} is the present value of paying the strike at expiry.
// Writing the formulas in terms of A and B (rather than S and K) is what makes
// the degenerate limits fall out cleanly: the whole option depends on S and K
// only through A and B, and the payoff kink sits exactly at A == B.
struct Ctx {
    double A;
    double B;
    double sqrtT;
    double d1;
    double d2;
    bool degenerate;  // total variance sigma^2*T is zero, or S == 0, or K == 0
};

Ctx context(double S, double K, double r, double T, double sigma, double q) noexcept {
    Ctx c{};
    c.A = S * std::exp(-q * T);
    c.B = K * std::exp(-r * T);
    c.sqrtT = std::sqrt(T);
    c.degenerate = (T == 0.0) || (sigma == 0.0) || (S == 0.0) || (K == 0.0);
    if (c.degenerate) {
        // log(S/K) and/or the division by sigma*sqrt(T) are undefined here.
        c.d1 = c.d2 = kNaN;
    } else {
        c.d1 = (std::log(S / K) + (r - q + 0.5 * sigma * sigma) * T) / (sigma * c.sqrtT);
        c.d2 = c.d1 - sigma * c.sqrtT;
    }
    return c;
}

// In the degenerate limit both N(d1) and N(d2) converge to the indicator of
// {A > B}: the terminal spot is a point mass at the forward, so the option
// either finishes in the money or it does not.
//
// Exactly on the kink (A == B) both d1 and d2 tend to 0 -- letting sigma -> 0
// with A == B gives d1 = +sigma*sqrt(T)/2 and d2 = -sigma*sqrt(T)/2, both of
// which vanish -- so the limit is N(0) = 0.5, not 0 or 1.
double indicator(const Ctx& c) noexcept {
    if (c.A > c.B) return 1.0;
    if (c.A < c.B) return 0.0;
    return 0.5;
}

// True on the payoff kink, where the value function is not differentiable.
// S == 0 or K == 0 is excluded: there the payoff is linear, not kinked.
bool on_kink(const Ctx& c, double S, double K) noexcept {
    return S > 0.0 && K > 0.0 && c.A == c.B;
}

}  // namespace

double BlackScholes::norm_cdf(double x) noexcept {
    return 0.5 * std::erfc(-x / std::sqrt(2.0));
}

double BlackScholes::norm_pdf(double x) noexcept {
    return std::exp(-0.5 * x * x) / std::sqrt(2.0 * M_PI);
}

// ---------------------------------------------------------------------------
// Pricing
// ---------------------------------------------------------------------------

double BlackScholes::call_price(double S, double K, double r, double T, double sigma, double q) noexcept {
    if (invalid(S, K, r, T, sigma, q)) return kNaN;
    const Ctx c = context(S, K, r, T, sigma, q);
    // Zero variance => the spot is worth its forward with certainty, so the
    // option is worth its discounted intrinsic value.  This single expression
    // also covers S == 0 (A == 0, call worthless) and K == 0 (B == 0, the call
    // degenerates into the forward itself).
    if (c.degenerate) return std::max(c.A - c.B, 0.0);
    return c.A * norm_cdf(c.d1) - c.B * norm_cdf(c.d2);
}

double BlackScholes::put_price(double S, double K, double r, double T, double sigma, double q) noexcept {
    if (invalid(S, K, r, T, sigma, q)) return kNaN;
    const Ctx c = context(S, K, r, T, sigma, q);
    if (c.degenerate) return std::max(c.B - c.A, 0.0);
    return c.B * norm_cdf(-c.d2) - c.A * norm_cdf(-c.d1);
}

// ---------------------------------------------------------------------------
// Greeks
// ---------------------------------------------------------------------------

double BlackScholes::call_delta(double S, double K, double r, double T, double sigma, double q) noexcept {
    if (invalid(S, K, r, T, sigma, q)) return kNaN;
    const Ctx c = context(S, K, r, T, sigma, q);
    const double dfq = std::exp(-q * T);
    if (c.degenerate) return dfq * indicator(c);
    return dfq * norm_cdf(c.d1);
}

double BlackScholes::put_delta(double S, double K, double r, double T, double sigma, double q) noexcept {
    if (invalid(S, K, r, T, sigma, q)) return kNaN;
    const Ctx c = context(S, K, r, T, sigma, q);
    const double dfq = std::exp(-q * T);
    // put delta = -e^{-qT} N(-d1) = e^{-qT} (N(d1) - 1), so it stays exactly
    // one e^{-qT} below the call delta -- that is delta parity with dividends.
    if (c.degenerate) return dfq * (indicator(c) - 1.0);
    return dfq * (norm_cdf(c.d1) - 1.0);
}

double BlackScholes::gamma(double S, double K, double r, double T, double sigma, double q) noexcept {
    if (invalid(S, K, r, T, sigma, q)) return kNaN;
    const Ctx c = context(S, K, r, T, sigma, q);
    if (c.degenerate) {
        // With no variance left the value is piecewise linear in S, so its
        // second derivative is zero away from the kink and a Dirac mass on it.
        // +infinity is the honest limit; it is also exactly what "gamma blows
        // up as an ATM option approaches expiry" means.
        return on_kink(c, S, K) ? kInf : 0.0;
    }
    return std::exp(-q * T) * norm_pdf(c.d1) / (S * sigma * c.sqrtT);
}

double BlackScholes::vega(double S, double K, double r, double T, double sigma, double q) noexcept {
    if (invalid(S, K, r, T, sigma, q)) return kNaN;
    const Ctx c = context(S, K, r, T, sigma, q);
    if (c.degenerate) {
        // At expiry (T == 0) vega is 0: there is no time left for vol to act.
        // At sigma == 0 with T > 0 it is 0 too, except exactly on the kink,
        // where d1 -> 0 and the limit is A*sqrt(T)*phi(0) > 0: a straddle
        // struck at the forward has vega even in the limit of zero vol.
        // sqrtT == 0 makes the T == 0 case fall out of the same expression.
        return on_kink(c, S, K) ? c.A * c.sqrtT * norm_pdf(0.0) : 0.0;
    }
    return c.A * norm_pdf(c.d1) * c.sqrtT;
}

double BlackScholes::call_theta(double S, double K, double r, double T, double sigma, double q) noexcept {
    if (invalid(S, K, r, T, sigma, q)) return kNaN;
    const Ctx c = context(S, K, r, T, sigma, q);
    if (c.degenerate) {
        // An ATM option struck at the forward decays infinitely fast at the
        // instant of expiry: the diffusion term -A*phi(d1)*sigma/(2*sqrt(T))
        // diverges as T -> 0 with sigma > 0.
        if (on_kink(c, S, K) && T == 0.0 && sigma > 0.0) return -kInf;
        // Otherwise the diffusion term vanishes and only carry is left.
        const double ind = indicator(c);
        return ind * (q * c.A - r * c.B);
    }
    const double diffusion = -c.A * norm_pdf(c.d1) * sigma / (2.0 * c.sqrtT);
    const double carry = q * c.A * norm_cdf(c.d1) - r * c.B * norm_cdf(c.d2);
    return diffusion + carry;
}

double BlackScholes::put_theta(double S, double K, double r, double T, double sigma, double q) noexcept {
    if (invalid(S, K, r, T, sigma, q)) return kNaN;
    const Ctx c = context(S, K, r, T, sigma, q);
    if (c.degenerate) {
        if (on_kink(c, S, K) && T == 0.0 && sigma > 0.0) return -kInf;
        const double ind = indicator(c);
        return (1.0 - ind) * (r * c.B - q * c.A);
    }
    const double diffusion = -c.A * norm_pdf(c.d1) * sigma / (2.0 * c.sqrtT);
    const double carry = -q * c.A * norm_cdf(-c.d1) + r * c.B * norm_cdf(-c.d2);
    return diffusion + carry;
}

double BlackScholes::call_rho(double S, double K, double r, double T, double sigma, double q) noexcept {
    if (invalid(S, K, r, T, sigma, q)) return kNaN;
    const Ctx c = context(S, K, r, T, sigma, q);
    // rho = K*T*e^{-rT}*N(d2) = T*B*N(d2).  T == 0 sends this to 0 on its own.
    if (c.degenerate) return T * c.B * indicator(c);
    return T * c.B * norm_cdf(c.d2);
}

double BlackScholes::put_rho(double S, double K, double r, double T, double sigma, double q) noexcept {
    if (invalid(S, K, r, T, sigma, q)) return kNaN;
    const Ctx c = context(S, K, r, T, sigma, q);
    if (c.degenerate) return -T * c.B * (1.0 - indicator(c));
    return -T * c.B * norm_cdf(-c.d2);
}

// ---------------------------------------------------------------------------
// Implied volatility
// ---------------------------------------------------------------------------

namespace {

// Refuse to report a volatility above 1000%.  Beyond this the price is a
// rounding artefact and any number we returned would be invented.
constexpr double kMaxSigma = 10.0;

// Bracket width in vol at which we stop.  This is the accuracy of the answer,
// which is why we converge on it rather than on an absolute price tolerance:
// a price tolerance of 1e-13 is meaningless for an option worth 1e-23, and it
// would let the solver exit immediately with any sigma at all.
constexpr double kSigmaTol = 1e-12;

// Below this, the Newton step price_error/vega is numerically meaningless.
constexpr double kVegaFloor = 1e-12;

constexpr int kMaxIter = 200;

double implied_vol(double price, double S, double K, double r, double T, double q,
                   bool is_call) noexcept {
    if (!std::isfinite(price) || !std::isfinite(S) || !std::isfinite(K) ||
        !std::isfinite(r) || !std::isfinite(T) || !std::isfinite(q)) {
        return kNaN;
    }
    if (S <= 0.0 || K <= 0.0 || T <= 0.0) return kNaN;

    const double A = S * std::exp(-q * T);
    const double B = K * std::exp(-r * T);

    // Invert the out-of-the-money twin.  C - P = A - B is exact, so the twin
    // has the same implied vol, but its price is pure time value rather than
    // time value hidden under a mountain of intrinsic.  See the header.
    if (is_call && A > B) {
        price = price - (A - B);
        is_call = false;
    } else if (!is_call && B > A) {
        price = price + (A - B);
        is_call = true;
    }

    // After the mapping the option is out of the money, so its intrinsic value
    // is 0 and the no-arbitrage band is simply [0, A) for a call, [0, B) for a
    // put.  These two tests are exactly the original bounds, carried across.
    const double upper = is_call ? A : B;
    if (!(price >= 0.0)) return kNaN;  // below intrinsic (also catches NaN)
    if (price >= upper) return kNaN;   // no finite sigma reaches the upper bound
    if (price == 0.0) return 0.0;      // exactly the zero-volatility price

    const auto reprice = [&](double v) noexcept {
        return is_call ? BlackScholes::call_price(S, K, r, T, v, q)
                       : BlackScholes::put_price(S, K, r, T, v, q);
    };

    // The BS price is strictly increasing in sigma, from the intrinsic value at
    // sigma = 0 to `upper` as sigma -> infinity.  Grow the upper end of the
    // bracket until it dominates the target price.
    double lo = 0.0;
    double hi = 1.0;
    while (reprice(hi) < price) {
        if (hi >= kMaxSigma) return kNaN;  // price implies vol above the cap
        hi = std::fmin(hi * 2.0, kMaxSigma);
    }

    // Brenner-Subrahmanyam seed: sigma ~ sqrt(2*pi/T) * price / A.  Exact for an
    // at-the-money-forward option, a decent starting point elsewhere, and if it
    // lands outside the bracket we just start from the midpoint.
    double v = std::sqrt(2.0 * M_PI / T) * price / A;
    if (!(v > lo && v < hi)) v = 0.5 * (lo + hi);

    for (int i = 0; i < kMaxIter; ++i) {
        const double p = reprice(v);

        // Maintain the bracket. Monotonicity in sigma is what makes this valid.
        if (p > price) {
            hi = v;
        } else if (p < price) {
            lo = v;
        } else {
            return v;  // exact hit
        }
        if (hi - lo <= kSigmaTol) return 0.5 * (lo + hi);

        const double vg = BlackScholes::vega(S, K, r, T, v, q);

        // Newton step, unless vega has collapsed (the wings).
        double next = (vg > kVegaFloor) ? v - (p - price) / vg : 0.5 * (lo + hi);

        // ...and unless Newton wants to leave the bracket. Either way: bisect.
        if (!std::isfinite(next) || next <= lo || next >= hi) {
            next = 0.5 * (lo + hi);
        }
        v = next;
    }
    // kMaxIter bisections alone would shrink the bracket by 2^-200, so reaching
    // here means the price is flat in sigma to machine precision. The midpoint
    // is the best available answer and the bracket bounds it.
    return 0.5 * (lo + hi);
}

}  // namespace

double BlackScholes::call_implied_vol(double price, double S, double K, double r, double T, double q) noexcept {
    return implied_vol(price, S, K, r, T, q, /*is_call=*/true);
}

double BlackScholes::put_implied_vol(double price, double S, double K, double r, double T, double q) noexcept {
    return implied_vol(price, S, K, r, T, q, /*is_call=*/false);
}
