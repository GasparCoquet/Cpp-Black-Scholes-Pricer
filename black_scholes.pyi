"""Type stubs for black_scholes C++ extension module."""

def norm_cdf(x: float) -> float:
    """Cumulative distribution function of the standard normal distribution."""
    ...

def call_price(S: float, K: float, r: float, T: float, sigma: float) -> float:
    """
    Calculate Black-Scholes call option price.
    
    Args:
        S: Spot price
        K: Strike price
        r: Risk-free rate
        T: Time to maturity (years)
        sigma: Volatility
    
    Returns:
        Call option price
    """
    ...

def put_price(S: float, K: float, r: float, T: float, sigma: float) -> float:
    """Calculate Black-Scholes put option price."""
    ...

def call_delta(S: float, K: float, r: float, T: float, sigma: float) -> float:
    """Calculate call option delta."""
    ...

def put_delta(S: float, K: float, r: float, T: float, sigma: float) -> float:
    """Calculate put option delta."""
    ...

def gamma(S: float, K: float, r: float, T: float, sigma: float) -> float:
    """Calculate option gamma (identical for calls and puts)."""
    ...

def vega(S: float, K: float, r: float, T: float, sigma: float) -> float:
    """Calculate option vega (identical for calls and puts)."""
    ...

def call_theta(S: float, K: float, r: float, T: float, sigma: float) -> float:
    """Calculate call option theta."""
    ...

def put_theta(S: float, K: float, r: float, T: float, sigma: float) -> float:
    """Calculate put option theta."""
    ...

def call_rho(S: float, K: float, r: float, T: float, sigma: float) -> float:
    """Calculate call option rho."""
    ...

def put_rho(S: float, K: float, r: float, T: float, sigma: float) -> float:
    """Calculate put option rho."""
    ...
