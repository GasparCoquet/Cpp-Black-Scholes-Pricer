"""Type stubs for the black_scholes C++ extension module.

All pricing and Greek functions take an optional continuous dividend yield q,
which defaults to 0.0 (plain Black-Scholes).  Set q = r to price options on a
future (Black-76), or q = the foreign rate for FX.

Theta is dV/dt per year.  Vega is dV/dsigma per 1.00 of vol.  Rho is dV/dr per
1.00 of rate.
"""

def norm_cdf(x: float) -> float:
    """Cumulative distribution function of the standard normal distribution."""
    ...

def norm_pdf(x: float) -> float:
    """Probability density function of the standard normal distribution."""
    ...

def call_price(S: float, K: float, r: float, T: float, sigma: float, q: float = 0.0) -> float:
    """
    Black-Scholes-Merton European call price.

    Args:
        S: Spot price (>= 0)
        K: Strike price (>= 0)
        r: Continuously-compounded risk-free rate
        T: Time to maturity in years (>= 0)
        sigma: Annualised volatility (>= 0)
        q: Continuous dividend yield (default 0.0)

    Returns:
        Call option price.  Degenerate inputs (T == 0, sigma == 0, S == 0,
        K == 0) return the correct limiting value -- the discounted intrinsic
        value -- rather than NaN.  Meaningless inputs (negative S, K, T or
        sigma) return NaN.
    """
    ...

def put_price(S: float, K: float, r: float, T: float, sigma: float, q: float = 0.0) -> float:
    """Black-Scholes-Merton European put price."""
    ...

def call_delta(S: float, K: float, r: float, T: float, sigma: float, q: float = 0.0) -> float:
    """Call delta, dC/dS."""
    ...

def put_delta(S: float, K: float, r: float, T: float, sigma: float, q: float = 0.0) -> float:
    """Put delta, dP/dS."""
    ...

def gamma(S: float, K: float, r: float, T: float, sigma: float, q: float = 0.0) -> float:
    """Gamma, d2V/dS2 (identical for calls and puts)."""
    ...

def vega(S: float, K: float, r: float, T: float, sigma: float, q: float = 0.0) -> float:
    """Vega, dV/dsigma per 1.00 of vol (identical for calls and puts)."""
    ...

def call_theta(S: float, K: float, r: float, T: float, sigma: float, q: float = 0.0) -> float:
    """Call theta, dC/dt per year."""
    ...

def put_theta(S: float, K: float, r: float, T: float, sigma: float, q: float = 0.0) -> float:
    """Put theta, dP/dt per year."""
    ...

def call_rho(S: float, K: float, r: float, T: float, sigma: float, q: float = 0.0) -> float:
    """Call rho, dC/dr per 1.00 of rate."""
    ...

def put_rho(S: float, K: float, r: float, T: float, sigma: float, q: float = 0.0) -> float:
    """Put rho, dP/dr per 1.00 of rate."""
    ...

def call_implied_vol(price: float, S: float, K: float, r: float, T: float, q: float = 0.0) -> float:
    """
    Implied volatility of a European call.

    Newton-Raphson on vega, guarded by a bracket: when vega collapses on the
    wings, or a Newton step would leave the bracket, the solver takes a
    bisection step instead.  Internally it inverts the out-of-the-money twin
    (mapped across with put-call parity), whose price is pure time value and is
    therefore far better conditioned.

    Returns NaN if no volatility could have produced this price -- i.e. if it
    violates the no-arbitrage bounds
        max(S*e^-qT - K*e^-rT, 0) <= price < S*e^-qT
    or if it implies a volatility above 1000%.  A price exactly on the lower
    bound returns 0.0.
    """
    ...

def put_implied_vol(price: float, S: float, K: float, r: float, T: float, q: float = 0.0) -> float:
    """
    Implied volatility of a European put.  See call_implied_vol.

    Returns NaN if the price violates the no-arbitrage bounds
        max(K*e^-rT - S*e^-qT, 0) <= price < K*e^-rT
    or implies a volatility above 1000%.
    """
    ...
