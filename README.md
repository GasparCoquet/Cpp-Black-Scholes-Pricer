# cpp-black-scholes-pricer

High-performance C++ implementation of the Black-Scholes model for pricing European options and computing Greeks (Delta, Gamma, Vega, Theta, Rho).

## Features

- **Option Pricing**: Call and Put prices using the Black-Scholes formula
- **Greeks Calculation**: Delta, Gamma, Vega, Theta, Rho for risk management
- **Performance**: Optimized C++ implementation for speed
- **Testing**: Comprehensive unit tests for accuracy
- **Benchmarking**: Performance measurement tools

## Build Instructions

### Prerequisites
- C++ compiler with C++17 support (MSVC, GCC, or Clang)
- CMake 3.10 or higher

### Compile

```bash
mkdir build
cd build
cmake ..
cmake --build .
```

On Windows with Visual Studio, add the configuration flag:
```bash
cmake --build . --config Debug
```

## Usage

### Run the main program
```bash
# Linux/Mac
./cpp-black-scholes-pricer

# Windows
.\Debug\cpp-black-scholes-pricer.exe
```

Output:
```
Call Price: 10.4506
Put Price:  5.57353
```

### Run benchmarks
```bash
# Linux/Mac
./benchmark

# Windows
.\Debug\benchmark.exe
```

The benchmark prices 1,000,000 random options and compares single-threaded vs multi-threaded execution (using hardware_concurrency) and prints the speedup.

## Testing

Run tests from the build directory:

```bash
# Linux/Mac
ctest

# Windows
ctest -C Debug
```

Or run directly:
```bash
# Windows
.\Debug\test_greeks.exe
```

### Test Coverage

The test suite includes 13 comprehensive tests:
- **Pricing**: Call and Put prices against reference values
- **Greeks**: Delta, Gamma, Vega, Theta, Rho calculations
- **Relationships**: Delta parity (Call Δ - Put Δ = 1)
- **Edge Cases**: Deep in-the-money, deep out-of-the-money, near expiry scenarios

## API Example

```cpp
#include "BlackScholes.h"

double S = 100;      // Spot price
double K = 100;      // Strike price
double r = 0.05;     // Risk-free rate (5%)
double T = 1.0;      // Time to maturity (1 year)
double sigma = 0.2;  // Volatility (20%)

// Pricing
double call = BlackScholes::call_price(S, K, r, T, sigma);
double put = BlackScholes::put_price(S, K, r, T, sigma);

// Greeks
double delta = BlackScholes::call_delta(S, K, r, T, sigma);
double gamma = BlackScholes::gamma(S, K, r, T, sigma);
double vega = BlackScholes::vega(S, K, r, T, sigma);
```

## Python Bindings (Pybind11)

### Installation

Install the Python module using pip:

```bash
pip install pybind11
pip install .
```

Or for development mode:

```bash
pip install -e .
```

### Python Usage

```python
import black_scholes as bs

# Market parameters
S = 100.0      # Spot price
K = 100.0      # Strike price
r = 0.05       # Risk-free rate (5%)
T = 1.0        # Time to maturity (1 year)
sigma = 0.2    # Volatility (20%)

# Calculate option prices
call_price = bs.call_price(S, K, r, T, sigma)
put_price = bs.put_price(S, K, r, T, sigma)

# Calculate Greeks
delta = bs.call_delta(S, K, r, T, sigma)
gamma = bs.gamma(S, K, r, T, sigma)
vega = bs.vega(S, K, r, T, sigma)
theta = bs.call_theta(S, K, r, T, sigma)
rho = bs.call_rho(S, K, r, T, sigma)

print(f"Call Price: {call_price:.4f}")
print(f"Delta: {delta:.6f}")
```

### Run Python Example

```bash
python examples/python_example.py
```

### Performance Benefits

Python bindings provide **10-100x speedup** over pure Python implementations:
- C++ compiled code
- No Python overhead
- Perfect for high-frequency calculations
- Ideal for backtesting and real-time pricing

## Project Structure

```
cpp-black-scholes-pricer/
├── src/
│   ├── BlackScholes.h        # Header file
│   ├── BlackScholes.cpp      # Implementation
│   ├── bindings.cpp          # Pybind11 bindings
│   ├── main.cpp              # Demo program
│   └── main_benchmark.cpp    # Benchmark program
├── tests/
│   └── test_greeks.cpp       # Unit tests
├── examples/
│   └── python_example.py     # Python usage example
├── CMakeLists.txt            # Build configuration
├── setup.py                  # Python package setup
└── README.md
```

## License

See [LICENSE](LICENSE) for details.
