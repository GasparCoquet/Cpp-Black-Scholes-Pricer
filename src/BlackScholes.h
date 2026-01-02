#pragma once
#include <cmath>

class BlackScholes {
public:
    [[nodiscard]] static double norm_cdf(double x) noexcept;
    [[nodiscard]] static double call_price(double S, double K, double r, double T, double sigma) noexcept;
    [[nodiscard]] static double put_price(double S, double K, double r, double T, double sigma) noexcept;
    [[nodiscard]] static double call_delta(double S, double K, double r, double T, double sigma) noexcept;
    [[nodiscard]] static double put_delta(double S, double K, double r, double T, double sigma) noexcept;
    [[nodiscard]] static double gamma(double S, double K, double r, double T, double sigma) noexcept;
    [[nodiscard]] static double vega(double S, double K, double r, double T, double sigma) noexcept;
    [[nodiscard]] static double call_theta(double S, double K, double r, double T, double sigma) noexcept;
    [[nodiscard]] static double put_theta(double S, double K, double r, double T, double sigma) noexcept;
    [[nodiscard]] static double call_rho(double S, double K, double r, double T, double sigma) noexcept;
    [[nodiscard]] static double put_rho(double S, double K, double r, double T, double sigma) noexcept;
};
