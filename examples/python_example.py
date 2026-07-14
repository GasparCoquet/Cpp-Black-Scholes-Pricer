"""
Example usage of the Black-Scholes Python bindings
"""
import math
import sys

import black_scholes as bs

# This script prints Greek letters (sigma, Delta, Gamma, ...).  A Windows
# console defaults to a legacy code page (cp1252) that cannot encode them, so
# the script died with UnicodeEncodeError before printing a single number.
if hasattr(sys.stdout, "reconfigure"):
    sys.stdout.reconfigure(encoding="utf-8", errors="replace")


def main():
    # Market parameters
    S = 100.0      # Spot price
    K = 100.0      # Strike price
    r = 0.05       # Risk-free rate (5%)
    T = 1.0        # Time to maturity (1 year)
    sigma = 0.2    # Volatility (20%)

    print("=" * 60)
    print("Black-Scholes Option Pricing (Python)")
    print("=" * 60)
    print(f"\nMarket Parameters:")
    print(f"  Spot Price (S):     ${S:.2f}")
    print(f"  Strike Price (K):   ${K:.2f}")
    print(f"  Risk-free Rate (r): {r*100:.2f}%")
    print(f"  Time to Maturity:   {T:.2f} years")
    print(f"  Volatility (σ):     {sigma*100:.2f}%")

    # Calculate prices
    call_price = bs.call_price(S, K, r, T, sigma)
    put_price = bs.put_price(S, K, r, T, sigma)

    print(f"\n{'Option Prices':^60}")
    print("-" * 60)
    print(f"  Call Price: ${call_price:.4f}")
    print(f"  Put Price:  ${put_price:.4f}")

    # Calculate Greeks
    print(f"\n{'Greeks':^60}")
    print("-" * 60)
    
    print(f"\nCall Option Greeks:")
    print(f"  Delta (Δ):  {bs.call_delta(S, K, r, T, sigma):.6f}")
    print(f"  Gamma (Γ):  {bs.gamma(S, K, r, T, sigma):.6f}")
    print(f"  Vega (ν):   {bs.vega(S, K, r, T, sigma):.6f}")
    print(f"  Theta (Θ):  {bs.call_theta(S, K, r, T, sigma):.6f}")
    print(f"  Rho (ρ):    {bs.call_rho(S, K, r, T, sigma):.6f}")

    print(f"\nPut Option Greeks:")
    print(f"  Delta (Δ):  {bs.put_delta(S, K, r, T, sigma):.6f}")
    print(f"  Gamma (Γ):  {bs.gamma(S, K, r, T, sigma):.6f}")
    print(f"  Vega (ν):   {bs.vega(S, K, r, T, sigma):.6f}")
    print(f"  Theta (Θ):  {bs.put_theta(S, K, r, T, sigma):.6f}")
    print(f"  Rho (ρ):    {bs.put_rho(S, K, r, T, sigma):.6f}")

    # Verify put-call parity.
    #
    # This is an assertion, not a printout.  Printing two numbers next to each
    # other and eyeballing them is not a verification: if they disagreed, the
    # script would still exit 0.  C - P = S - Ke^(-rT) is an exact identity for
    # European options (it is a static replication argument, not an
    # approximation), so it should hold to floating-point precision.
    print(f"\n{'Put-Call Parity Verification':^60}")
    print("-" * 60)
    lhs = call_price - put_price
    rhs = S - K * math.exp(-r * T)
    print(f"  C - P        = {lhs:.12f}")
    print(f"  S - Ke^(-rT) = {rhs:.12f}")
    print(f"  |difference| = {abs(lhs - rhs):.3e}")
    assert abs(lhs - rhs) < 1e-10, f"put-call parity violated by {abs(lhs - rhs)}"
    print("  OK: parity holds to 1e-10")

    # Delta parity: dC/dS - dP/dS = 1, again an exact identity.
    call_delta = bs.call_delta(S, K, r, T, sigma)
    put_delta = bs.put_delta(S, K, r, T, sigma)
    print(f"\n{'Delta Parity':^60}")
    print("-" * 60)
    print(f"  Call Delta - Put Delta = {call_delta - put_delta:.12f}")
    print(f"  Expected               = 1.000000000000")
    assert abs((call_delta - put_delta) - 1.0) < 1e-10, "delta parity violated"
    print("  OK: delta parity holds to 1e-10")

    # Implied volatility: invert the price back to the vol that produced it.
    print(f"\n{'Implied Volatility (round trip)':^60}")
    print("-" * 60)
    iv_call = bs.call_implied_vol(call_price, S, K, r, T)
    iv_put = bs.put_implied_vol(put_price, S, K, r, T)
    print(f"  input sigma        = {sigma:.12f}")
    print(f"  IV from call price = {iv_call:.12f}   (|err| {abs(iv_call - sigma):.2e})")
    print(f"  IV from put price  = {iv_put:.12f}   (|err| {abs(iv_put - sigma):.2e})")
    assert abs(iv_call - sigma) < 1e-8, f"call IV round trip failed: {iv_call}"
    assert abs(iv_put - sigma) < 1e-8, f"put IV round trip failed: {iv_put}"
    print("  OK: both round trip to within 1e-8")

    # A price no volatility could produce must come back NaN, not a number.
    impossible = bs.call_implied_vol(S + 1.0, S, K, r, T)  # above the upper bound
    print(f"  IV of an arbitrageable price = {impossible}  (correctly not a number)")
    assert math.isnan(impossible), "an impossible price should not yield an IV"

    # Dividend yield.
    print(f"\n{'Dividend Yield (q = 3%)':^60}")
    print("-" * 60)
    q = 0.03
    c_q = bs.call_price(S, K, r, T, sigma, q)
    p_q = bs.put_price(S, K, r, T, sigma, q)
    print(f"  Call Price: ${c_q:.4f}   (vs ${call_price:.4f} with no dividend)")
    print(f"  Put Price:  ${p_q:.4f}   (vs ${put_price:.4f} with no dividend)")
    # Parity with dividends: C - P = S*e^(-qT) - K*e^(-rT).
    rhs_q = S * math.exp(-q * T) - K * math.exp(-r * T)
    assert abs((c_q - p_q) - rhs_q) < 1e-10, "parity with dividends violated"
    print(f"  C - P = S*e^(-qT) - K*e^(-rT) holds to 1e-10")
    assert abs(bs.call_implied_vol(c_q, S, K, r, T, q) - sigma) < 1e-8
    print("  OK: IV round trip with dividends holds to 1e-8")

    # Degenerate inputs return their limiting value, not a silent NaN.
    print(f"\n{'Degenerate Inputs (used to return NaN)':^60}")
    print("-" * 60)
    print(f"  call(S=120, K=100, T=0)     = {bs.call_price(120, 100, r, 0.0, sigma):.4f}  (intrinsic)")
    print(f"  put (S=120, K=100, T=0)     = {bs.put_price(120, 100, r, 0.0, sigma):.4f}")
    print(f"  call(S=100, K=100, sigma=0) = {bs.call_price(100, 100, r, T, 0.0):.4f}")
    print(f"  put (S=0,   K=100)          = {bs.put_price(0.0, 100, r, T, sigma):.4f}  (= K*e^(-rT))")
    for label, value in [
        ("T=0", bs.call_price(120, 100, r, 0.0, sigma)),
        ("sigma=0", bs.call_price(100, 100, r, T, 0.0)),
        ("S=0", bs.put_price(0.0, 100, r, T, sigma)),
    ]:
        assert not math.isnan(value), f"{label} still returns NaN"
    assert abs(bs.call_price(120, 100, r, 0.0, sigma) - 20.0) < 1e-12
    assert abs(bs.put_price(0.0, 100, r, T, sigma) - 100 * math.exp(-r * T)) < 1e-12
    print("  OK: none of these are NaN, and each equals its limiting value")

    print("\n" + "=" * 60)


def benchmark():
    """Quick performance test"""
    import time
    
    S, K, r, T, sigma = 100.0, 100.0, 0.05, 1.0, 0.2
    
    n = 1_000_000
    start = time.perf_counter()
    
    for _ in range(n):
        bs.call_price(S, K, r, T, sigma)
    
    elapsed = time.perf_counter() - start
    
    print(f"\nPerformance Benchmark:")
    print(f"  {n:,} call price calculations")
    print(f"  Time: {elapsed:.4f} seconds")
    print(f"  Rate: {n/elapsed:,.0f} calculations/second")


if __name__ == "__main__":
    main()
    benchmark()
