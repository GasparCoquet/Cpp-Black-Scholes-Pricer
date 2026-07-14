#include <pybind11/pybind11.h>
#include "BlackScholes.h"

namespace py = pybind11;

// Every pricing/Greek function takes an optional continuous dividend yield q,
// defaulting to 0.0 so existing five-argument Python calls keep working.
#define BS_ARGS                                                                \
    py::arg("S"), py::arg("K"), py::arg("r"), py::arg("T"), py::arg("sigma"),  \
        py::arg("q") = 0.0

// The implied-vol solvers take the observed price first, then the parameters.
#define BS_IV_ARGS                                                             \
    py::arg("price"), py::arg("S"), py::arg("K"), py::arg("r"), py::arg("T"),  \
        py::arg("q") = 0.0

PYBIND11_MODULE(black_scholes, m) {
    m.doc() = "Black-Scholes-Merton option pricing, Greeks and implied volatility";

    // Utility functions
    m.def("norm_cdf", &BlackScholes::norm_cdf,
          "Cumulative distribution function of the standard normal distribution",
          py::arg("x"));

    m.def("norm_pdf", &BlackScholes::norm_pdf,
          "Probability density function of the standard normal distribution",
          py::arg("x"));

    // Option pricing
    m.def("call_price", &BlackScholes::call_price,
          "European call price (Black-Scholes-Merton, continuous dividend yield q)",
          BS_ARGS);

    m.def("put_price", &BlackScholes::put_price,
          "European put price (Black-Scholes-Merton, continuous dividend yield q)",
          BS_ARGS);

    // Greeks - Delta
    m.def("call_delta", &BlackScholes::call_delta,
          "Delta of a call (dC/dS)", BS_ARGS);

    m.def("put_delta", &BlackScholes::put_delta,
          "Delta of a put (dP/dS)", BS_ARGS);

    // Greeks - Gamma
    m.def("gamma", &BlackScholes::gamma,
          "Gamma (d2V/dS2) - identical for calls and puts", BS_ARGS);

    // Greeks - Vega
    m.def("vega", &BlackScholes::vega,
          "Vega (dV/dsigma, per 1.00 of vol) - identical for calls and puts",
          BS_ARGS);

    // Greeks - Theta
    m.def("call_theta", &BlackScholes::call_theta,
          "Theta of a call (dC/dt, per year)", BS_ARGS);

    m.def("put_theta", &BlackScholes::put_theta,
          "Theta of a put (dP/dt, per year)", BS_ARGS);

    // Greeks - Rho
    m.def("call_rho", &BlackScholes::call_rho,
          "Rho of a call (dC/dr, per 1.00 of rate)", BS_ARGS);

    m.def("put_rho", &BlackScholes::put_rho,
          "Rho of a put (dP/dr, per 1.00 of rate)", BS_ARGS);

    // Implied volatility
    m.def("call_implied_vol", &BlackScholes::call_implied_vol,
          "Implied volatility of a call. Newton-Raphson with a vega guard and a "
          "bisection fallback; inverts the out-of-the-money twin via put-call "
          "parity for conditioning. Returns NaN if no volatility could have "
          "produced this price (no-arbitrage bounds violated, or IV > 1000%).",
          BS_IV_ARGS);

    m.def("put_implied_vol", &BlackScholes::put_implied_vol,
          "Implied volatility of a put. Newton-Raphson with a vega guard and a "
          "bisection fallback; inverts the out-of-the-money twin via put-call "
          "parity for conditioning. Returns NaN if no volatility could have "
          "produced this price (no-arbitrage bounds violated, or IV > 1000%).",
          BS_IV_ARGS);

    m.attr("__version__") = "2.0.0";
}
