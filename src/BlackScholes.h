#pragma once

// Black-Scholes-Merton pricing for European options on an asset paying a
// continuous dividend yield q.
//
// Conventions
// -----------
//   S      spot price of the underlying          (S >= 0)
//   K      strike                                (K >= 0)
//   r      continuously-compounded risk-free rate
//   T      time to expiry in years               (T >= 0)
//   sigma  annualised volatility                 (sigma >= 0)
//   q      continuous dividend yield             (default 0, so the old
//                                                 five-argument calls still
//                                                 compile and still mean the
//                                                 same thing)
//
// Setting q = 0 recovers plain Black-Scholes.  q is also how you price options
// on a futures contract (q = r, Black-76) or on FX (q = foreign rate).
//
// Theta is returned as dV/dt (per year), i.e. the negative of dV/dT.
// Vega is dV/dsigma (per 1.00 of vol, not per vol point).
// Rho  is dV/dr     (per 1.00 of rate).
//
// Degenerate inputs
// -----------------
// The closed form divides by sigma*sqrt(T) and takes log(S/K), so T = 0,
// sigma = 0, S = 0 or K = 0 all used to produce a silent NaN.  Every function
// below instead returns the correct limiting value.  When the total variance
// sigma^2 * T is zero the terminal spot is a point mass at the forward and the
// option is worth its discounted intrinsic value, max(S*e^{-qT} - K*e^{-rT}, 0)
// for a call.  That payoff is kinked, so gamma is a Dirac mass there: gamma and
// theta return +/-infinity exactly on the kink, which is the true limit, not a
// silent NaN.
//
// Genuinely meaningless inputs -- negative spot, negative strike, negative time
// to expiry, negative volatility, or any non-finite input -- return NaN. That
// is a caller bug rather than a limiting case, and NaN propagates loudly.

class BlackScholes {
public:
    // Standard normal CDF and PDF.
    [[nodiscard]] static double norm_cdf(double x) noexcept;
    [[nodiscard]] static double norm_pdf(double x) noexcept;

    // Pricing
    [[nodiscard]] static double call_price(double S, double K, double r, double T, double sigma, double q = 0.0) noexcept;
    [[nodiscard]] static double put_price(double S, double K, double r, double T, double sigma, double q = 0.0) noexcept;

    // Greeks
    [[nodiscard]] static double call_delta(double S, double K, double r, double T, double sigma, double q = 0.0) noexcept;
    [[nodiscard]] static double put_delta(double S, double K, double r, double T, double sigma, double q = 0.0) noexcept;
    [[nodiscard]] static double gamma(double S, double K, double r, double T, double sigma, double q = 0.0) noexcept;
    [[nodiscard]] static double vega(double S, double K, double r, double T, double sigma, double q = 0.0) noexcept;
    [[nodiscard]] static double call_theta(double S, double K, double r, double T, double sigma, double q = 0.0) noexcept;
    [[nodiscard]] static double put_theta(double S, double K, double r, double T, double sigma, double q = 0.0) noexcept;
    [[nodiscard]] static double call_rho(double S, double K, double r, double T, double sigma, double q = 0.0) noexcept;
    [[nodiscard]] static double put_rho(double S, double K, double r, double T, double sigma, double q = 0.0) noexcept;

    // -----------------------------------------------------------------------
    // Implied volatility: the sigma that reprices `price`.
    //
    // Algorithm: Newton-Raphson, guarded by a bracket that only ever shrinks.
    // Vega is the Newton derivative.  Two things make plain Newton fail here,
    // and both are handled:
    //
    //   * Vega collapses to ~0 on the wings (deep ITM/OTM, short expiry), so
    //     the Newton step diff/vega explodes.  If vega falls below a floor, the
    //     solver takes a bisection step instead.
    //   * Even with healthy vega, a Newton step can jump outside the bracket
    //     (the price is convex in sigma).  Any step that leaves the bracket is
    //     replaced by a bisection step.
    //
    // So it keeps Newton's quadratic convergence where the function is nice and
    // degrades to bisection's guaranteed convergence where it is not.
    //
    // The solver inverts the OUT-OF-THE-MONEY twin of whatever it is handed,
    // mapping across with put-call parity (C - P = S*e^{-qT} - K*e^{-rT}, an
    // exact identity).  This matters: an in-the-money price is mostly intrinsic
    // value, and the vol-dependent part is buried underneath it.  A 250/100
    // call with 0.02y to expiry is worth 150.000000000152 -- the time value is
    // 12 orders of magnitude below the intrinsic, so a double holds only ~4
    // significant digits of it, and no solver can then recover sigma to better
    // than ~1e-5.  The out-of-the-money twin of the same option is worth
    // 1.5199e-10, which is pure time value at full precision.  Inverting that
    // recovers sigma to ~1e-12.
    //
    // Returns NaN, rather than a plausible-looking number, when no sigma could
    // have produced `price`:
    //     call:  max(A - B, 0) <= price < A
    //     put:   max(B - A, 0) <= price < B     [A = S*e^{-qT}, B = K*e^{-rT}]
    // and when the price implies a volatility above 1000%.  A garbage IV that
    // silently reads 0.31 is far more dangerous on a trading system than one
    // that is obviously NaN.  A price exactly on the lower bound returns 0.0.
    //
    // NaN is also returned when the option is so deep in the money that a double
    // cannot carry its time value underneath the intrinsic.  Inversion goes via
    // the out-of-the-money twin, and forming it costs a subtraction of two nearly
    // equal numbers: a 250/25 call at T = 0.02 and sigma = 2.0 differs from its
    // own intrinsic by 2.8e-14, below the rounding error of the operands.  Every
    // sigma from 0 to well past 1% then produces the same double, so no answer
    // exists to return: neither 0.0 nor the plausible number a solver run on
    // that residue would converge to.  Quote and invert the OTM wing instead.
    [[nodiscard]] static double call_implied_vol(double price, double S, double K, double r, double T, double q = 0.0) noexcept;
    [[nodiscard]] static double put_implied_vol(double price, double S, double K, double r, double T, double q = 0.0) noexcept;
};
