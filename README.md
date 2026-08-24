# cpp-black-scholes-pricer

C++ implementation of the Black-Scholes-Merton model: European option pricing,
Greeks, and implied volatility, with Python bindings.

## Features

- **Pricing**: European calls and puts, with a continuous dividend yield `q`.
  Setting `q = r` gives Black-76 (options on futures); setting `q` to the
  foreign rate gives FX options.
- **Greeks**: Delta, Gamma, Vega, Theta, Rho. Every one of them is checked
  against a central finite difference of the price over a 2160-point parameter
  grid, not just for sign.
- **Implied volatility**: Newton-Raphson with a vega guard and a bisection
  fallback. Returns NaN for prices no volatility could have produced, rather
  than a plausible-looking number.
- **Degenerate inputs**: `T = 0`, `sigma = 0`, `S = 0` and `K = 0` return their
  correct limiting values instead of a silent NaN.
- **Python bindings** via pybind11.

## Conventions

| Symbol | Meaning |
| --- | --- |
| `S` | Spot price (>= 0) |
| `K` | Strike (>= 0) |
| `r` | Continuously-compounded risk-free rate |
| `T` | Time to expiry in years (>= 0) |
| `sigma` | Annualised volatility (>= 0) |
| `q` | Continuous dividend yield (optional, defaults to 0) |

Theta is `dV/dt` per year (not `dV/dT`). Vega is `dV/dsigma` per 1.00 of vol,
not per vol point. Rho is `dV/dr` per 1.00 of rate.

Discounting is **continuous**: the put-call parity identity this library
satisfies is `C - P = S*e^(-qT) - K*e^(-rT)`.

## Build

### Prerequisites
- A C++17 compiler (GCC, Clang or MSVC)
- CMake 3.10+

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

## Usage

### Run the demo program

```bash
./build/cpp-black-scholes-pricer      # Windows: .\build\Release\cpp-black-scholes-pricer.exe
```

Output (S=100, K=100, r=5%, T=1y, sigma=20%). This block is the verbatim output
of the "Run the demo program" step of the CI build, so it cannot silently drift
from what the code does:

```
Call Price: 10.4506
Put Price:  5.57353
Call Delta: 0.636831
Put  Delta: -0.363169
Gamma:      0.018762
Vega:       37.524
Call Theta: -6.41403
Put  Theta: -1.65788
Call Rho:   53.2325
Put  Rho:   -41.8905

Implied vol from the call price above:
  input sigma:  0.20000000000000001
  recovered:    0.20000000000000173
  |error|:      1.7208456881689926e-15

With a 3% continuous dividend yield:
  Call Price: 8.65253
  Put Price:  6.73092
  Call Delta: 0.56214

At expiry (T = 0), an option is worth its intrinsic value:
  call(S=120, K=100, T=0): 20
  put (S=120, K=100, T=0): 0
```

### Run the benchmark

```bash
./build/benchmark
```

Prices 1,000,000 random options and compares single-threaded against
multi-threaded execution (`hardware_concurrency`), printing the speedup it
measures on your machine.

## API

```cpp
#include "BlackScholes.h"

const double S = 100, K = 100, r = 0.05, T = 1.0, sigma = 0.2;

// Pricing. The trailing dividend yield q is optional and defaults to 0.
double call = BlackScholes::call_price(S, K, r, T, sigma);
double put  = BlackScholes::put_price(S, K, r, T, sigma);
double call_with_div = BlackScholes::call_price(S, K, r, T, sigma, /*q=*/0.03);

// Greeks
double delta = BlackScholes::call_delta(S, K, r, T, sigma);
double gamma = BlackScholes::gamma(S, K, r, T, sigma);
double vega  = BlackScholes::vega(S, K, r, T, sigma);
double theta = BlackScholes::call_theta(S, K, r, T, sigma);
double rho   = BlackScholes::call_rho(S, K, r, T, sigma);

// Implied volatility: invert an observed price back to a vol.
double iv = BlackScholes::call_implied_vol(call, S, K, r, T);
```

### Implied volatility

`call_implied_vol` / `put_implied_vol` solve for the sigma that reprices an
observed premium. The iteration is Newton-Raphson on vega, guarded by a bracket
that only ever shrinks: whenever vega collapses (deep in- or out-of-the-money,
where the price is numerically flat in sigma) or a Newton step would jump
outside the bracket, the solver takes a bisection step instead. That keeps
Newton's fast convergence where the function is well behaved and bisection's
guaranteed convergence where it is not.

Internally it always inverts the **out-of-the-money** twin of whatever it is
handed, mapping across with put-call parity. An in-the-money price is mostly
intrinsic value, and the vol-dependent part is buried under it: a 250/100 call
with 0.02y to expiry is worth `150.000000000152`, so a `double` holds only about
four significant digits of its time value and no solver could then recover sigma
to better than ~1e-5. Its out-of-the-money twin is worth `1.5199e-10` -- pure
time value at full precision -- and inverts cleanly.

It returns **NaN**, not a number, when no volatility could have produced the
price:

```
call:  max(S*e^-qT - K*e^-rT, 0)  <=  price  <  S*e^-qT
put:   max(K*e^-rT - S*e^-qT, 0)  <=  price  <  K*e^-rT
```

and when the price implies a volatility above 1000%. A garbage IV that silently
reads 0.31 is far more dangerous than one that is obviously NaN.

**And when the map itself runs out of precision.** Inverting via the OTM twin
costs a subtraction of two nearly equal numbers, `price - (A - B)`. Deep enough
in the money that cancellation eats the whole time value:

| | price | intrinsic | time value left |
|---|---|---|---|
| 250/25 call, `T=0.02`, `sigma=2.0` | `225.049950033316691` | `225.049950033316662` | `2.8e-14` |
| 50/25 call, one day, `sigma=0.20` | `25.000000000000000` | `25.000000000000000` | `0.0` |

The noise floor on those operands is `5.0e-14`, so in both rows every surviving
digit is rounding error. The library detects this and returns NaN. It does not
return `0.0` for the second row: that is a 20-vol option, and reporting it as
zero vol is precisely the silent-garbage failure this section is about.

### Degenerate inputs

`T = 0` is not an exotic input -- it is every option on its expiry date -- and
`sigma = 0` is what a vol solver hands you when it hits its lower bound. Both,
along with `S = 0` and `K = 0`, used to divide by `sigma*sqrt(T)` or take
`log(0)` and return a silent NaN. They now return the correct limit:

| Input | Behaviour |
| --- | --- |
| `T = 0` | Intrinsic value. Delta is the in-the-money indicator (0.5 exactly at the money). |
| `sigma = 0` | Discounted intrinsic, `max(S*e^-qT - K*e^-rT, 0)` for a call. |
| `S = 0` | Call is worthless; the put is a certain payoff worth `K*e^-rT`. |
| `K = 0` | The call is the forward, `S*e^-qT`; the put is worthless. |
| On the expiry kink | Gamma is `+inf` and theta `-inf`. That is the true limit of a kinked payoff, and it is what "gamma explodes into expiry" means -- reported loudly, not as NaN. |
| `S < 0`, `K < 0`, `T < 0`, `sigma < 0`, or any non-finite input | NaN. These are caller bugs, not limiting cases. |

## Testing

```bash
ctest --test-dir build --output-on-failure --build-config Release
```

Three suites, **30,622 assertions**, all passing in CI:

| Suite | Checks | What it pins down |
| --- | --- | --- |
| `test_greeks` | 25,947 | Reference values to 1e-12; put-call and delta parity as exact identities; `q` as a spot haircut; **all eight Greeks against a central finite difference** over a 2160-point grid |
| `test_implied_vol` | 4,623 | Round trip `sigma -> price -> IV` over a 4500-point grid; the wings where unguarded Newton diverges; no-arbitrage prices returning NaN |
| `test_edge_cases` | 57 | `T=0`, `sigma=0`, `S=0`, `K=0` limits, most of them bit-exact; invalid inputs returning NaN |

The finite-difference comparison is the test that actually pins the Greeks.
`gamma > 0` and `theta < 0` pass just as happily for a gamma that is 10x too
large; differentiating the price numerically does not. It catches what sign
checks sail past: a dropped `e^-qT` on the spot leg, a factor of 2 in the theta
diffusion term, or theta returned as `dV/dT` instead of `dV/dt`.

### A note on assert()

These tests deliberately do not use `assert()`. CI builds with
`CMAKE_BUILD_TYPE=Release`, which puts `-DNDEBUG` on the command line, and
`<cassert>` then expands every `assert()` to `((void)0)`.

This repository previously had 14 bare `assert()` calls as its entire test
suite, so **none of them ran in CI**. The binary printed "All Black-Scholes
tests passed!" and returned 0 no matter what the pricer computed. `call_price()`
could have returned 999999 and the build would have stayed green.

`tests/check.h` provides `CHECK` / `CHECK_NEAR` / `CHECK_CLOSE_REL`, which are
ordinary code rather than macros keyed off `NDEBUG`. They survive `-O3 -DNDEBUG`,
print the failing file, line, actual value, expected value and tolerance, and
make the process exit nonzero. The CI job also runs each test binary directly
and prints how many assertions it evaluated, so a suite that silently checks
nothing is visible in the log.

## Python bindings

### Install

```bash
pip install pybind11
pip install .
```

### Use

```python
import black_scholes as bs

S, K, r, T, sigma = 100.0, 100.0, 0.05, 1.0, 0.2

call = bs.call_price(S, K, r, T, sigma)
put  = bs.put_price(S, K, r, T, sigma)

# Dividend yield is an optional trailing argument.
call_with_div = bs.call_price(S, K, r, T, sigma, q=0.03)

# Greeks
delta = bs.call_delta(S, K, r, T, sigma)
gamma = bs.gamma(S, K, r, T, sigma)
vega  = bs.vega(S, K, r, T, sigma)

# Implied volatility
iv = bs.call_implied_vol(call, S, K, r, T)
```

### Run the example

```bash
python examples/python_example.py
```

It asserts put-call parity, delta parity, the implied-vol round trip (with and
without dividends), that an arbitrageable price yields NaN, and that the
degenerate inputs return their limiting values. CI runs it on every push, so
these are enforced rather than printed and eyeballed.

## Project structure

```
cpp-black-scholes-pricer/
├── src/
│   ├── BlackScholes.h        # API and the conventions it follows
│   ├── BlackScholes.cpp      # Pricing, Greeks, implied vol
│   ├── bindings.cpp          # Pybind11 bindings
│   ├── main.cpp              # Demo program
│   └── main_benchmark.cpp    # Benchmark program
├── tests/
│   ├── check.h               # CHECK macros that survive NDEBUG
│   ├── test_greeks.cpp       # Reference values, parities, finite differences
│   ├── test_implied_vol.cpp  # Round trip, wings, no-arbitrage bounds
│   └── test_edge_cases.cpp   # T=0, sigma=0, S=0, K=0, invalid inputs
├── examples/
│   └── python_example.py     # Python usage, with assertions
├── CMakeLists.txt
├── setup.py
└── README.md
```

## License

See [LICENSE](LICENSE) for details.
