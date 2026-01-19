"""
Example usage of the Black-Scholes Python bindings
"""
import black_scholes as bs

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

    # Verify put-call parity
    print(f"\n{'Put-Call Parity Verification':^60}")
    print("-" * 60)
    lhs = call_price - put_price
    rhs = S - K * bs.norm_cdf(0) * (1 / (1 + r)) ** T  # Simplified
    print(f"  C - P = {lhs:.6f}")
    print(f"  S - Ke^(-rT) = {S - K * (1 / (1 + r)) ** T:.6f}")

    # Delta parity
    call_delta = bs.call_delta(S, K, r, T, sigma)
    put_delta = bs.put_delta(S, K, r, T, sigma)
    print(f"\n{'Delta Parity':^60}")
    print("-" * 60)
    print(f"  Call Delta - Put Delta = {call_delta - put_delta:.6f}")
    print(f"  Expected: 1.000000")

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
