#include <pybind11/pybind11.h>
#include "BlackScholes.h"

namespace py = pybind11;

PYBIND11_MODULE(black_scholes, m) {
    m.doc() = "High-performance Black-Scholes option pricing library";

    // Utility functions
    m.def("norm_cdf", &BlackScholes::norm_cdf,
          "Cumulative distribution function of the standard normal distribution",
          py::arg("x"));

    // Option pricing
    m.def("call_price", &BlackScholes::call_price,
          "Calculate European call option price using Black-Scholes formula",
          py::arg("S"), py::arg("K"), py::arg("r"), py::arg("T"), py::arg("sigma"));

    m.def("put_price", &BlackScholes::put_price,
          "Calculate European put option price using Black-Scholes formula",
          py::arg("S"), py::arg("K"), py::arg("r"), py::arg("T"), py::arg("sigma"));

    // Greeks - Delta
    m.def("call_delta", &BlackScholes::call_delta,
          "Calculate delta for a call option (∂C/∂S)",
          py::arg("S"), py::arg("K"), py::arg("r"), py::arg("T"), py::arg("sigma"));

    m.def("put_delta", &BlackScholes::put_delta,
          "Calculate delta for a put option (∂P/∂S)",
          py::arg("S"), py::arg("K"), py::arg("r"), py::arg("T"), py::arg("sigma"));

    // Greeks - Gamma
    m.def("gamma", &BlackScholes::gamma,
          "Calculate gamma for an option (∂²V/∂S²) - same for calls and puts",
          py::arg("S"), py::arg("K"), py::arg("r"), py::arg("T"), py::arg("sigma"));

    // Greeks - Vega
    m.def("vega", &BlackScholes::vega,
          "Calculate vega for an option (∂V/∂σ) - same for calls and puts",
          py::arg("S"), py::arg("K"), py::arg("r"), py::arg("T"), py::arg("sigma"));

    // Greeks - Theta
    m.def("call_theta", &BlackScholes::call_theta,
          "Calculate theta for a call option (∂C/∂T)",
          py::arg("S"), py::arg("K"), py::arg("r"), py::arg("T"), py::arg("sigma"));

    m.def("put_theta", &BlackScholes::put_theta,
          "Calculate theta for a put option (∂P/∂T)",
          py::arg("S"), py::arg("K"), py::arg("r"), py::arg("T"), py::arg("sigma"));

    // Greeks - Rho
    m.def("call_rho", &BlackScholes::call_rho,
          "Calculate rho for a call option (∂C/∂r)",
          py::arg("S"), py::arg("K"), py::arg("r"), py::arg("T"), py::arg("sigma"));

    m.def("put_rho", &BlackScholes::put_rho,
          "Calculate rho for a put option (∂P/∂r)",
          py::arg("S"), py::arg("K"), py::arg("r"), py::arg("T"), py::arg("sigma"));

    // Version info
    m.attr("__version__") = "1.0.0";
}
