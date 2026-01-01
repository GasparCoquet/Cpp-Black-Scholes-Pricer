#define _USE_MATH_DEFINES
#include "BlackScholes.h"
#include <cmath>

// Standard normal cumulative distribution function
// (using error function)
double BlackScholes::norm_cdf(double x) {
    return 0.5 * std::erfc(-x / std::sqrt(2));
}

double BlackScholes::call_price(double S, double K, double r, double T, double sigma) {
    double d1 = (std::log(S / K) + (r + 0.5 * sigma * sigma) * T) / (sigma * std::sqrt(T));
    double d2 = d1 - sigma * std::sqrt(T);
    return S * norm_cdf(d1) - K * std::exp(-r * T) * norm_cdf(d2);
}

double BlackScholes::put_price(double S, double K, double r, double T, double sigma) {
    double d1 = (std::log(S / K) + (r + 0.5 * sigma * sigma) * T) / (sigma * std::sqrt(T));
    double d2 = d1 - sigma * std::sqrt(T);
    return K * std::exp(-r * T) * norm_cdf(-d2) - S * norm_cdf(-d1);
}

// Standard normal PDF
static double norm_pdf(double x) {
    return std::exp(-0.5 * x * x) / std::sqrt(2 * M_PI);
}

double BlackScholes::call_delta(double S, double K, double r, double T, double sigma) {
    double d1 = (std::log(S / K) + (r + 0.5 * sigma * sigma) * T) / (sigma * std::sqrt(T));
    return norm_cdf(d1);
}

double BlackScholes::put_delta(double S, double K, double r, double T, double sigma) {
    double d1 = (std::log(S / K) + (r + 0.5 * sigma * sigma) * T) / (sigma * std::sqrt(T));
    return norm_cdf(d1) - 1.0;
}

double BlackScholes::gamma(double S, double K, double r, double T, double sigma) {
    double d1 = (std::log(S / K) + (r + 0.5 * sigma * sigma) * T) / (sigma * std::sqrt(T));
    return norm_pdf(d1) / (S * sigma * std::sqrt(T));
}

double BlackScholes::vega(double S, double K, double r, double T, double sigma) {
    double d1 = (std::log(S / K) + (r + 0.5 * sigma * sigma) * T) / (sigma * std::sqrt(T));
    return S * norm_pdf(d1) * std::sqrt(T);
}

double BlackScholes::call_theta(double S, double K, double r, double T, double sigma) {
    double d1 = (std::log(S / K) + (r + 0.5 * sigma * sigma) * T) / (sigma * std::sqrt(T));
    double d2 = d1 - sigma * std::sqrt(T);
    double term1 = -S * norm_pdf(d1) * sigma / (2 * std::sqrt(T));
    double term2 = r * K * std::exp(-r * T) * norm_cdf(d2);
    return term1 - term2;
}

double BlackScholes::put_theta(double S, double K, double r, double T, double sigma) {
    double d1 = (std::log(S / K) + (r + 0.5 * sigma * sigma) * T) / (sigma * std::sqrt(T));
    double d2 = d1 - sigma * std::sqrt(T);
    double term1 = -S * norm_pdf(d1) * sigma / (2 * std::sqrt(T));
    double term2 = r * K * std::exp(-r * T) * norm_cdf(-d2);
    return term1 + term2;
}

double BlackScholes::call_rho(double S, double K, double r, double T, double sigma) {
    double d1 = (std::log(S / K) + (r + 0.5 * sigma * sigma) * T) / (sigma * std::sqrt(T));
    double d2 = d1 - sigma * std::sqrt(T);
    return K * T * std::exp(-r * T) * norm_cdf(d2);
}

double BlackScholes::put_rho(double S, double K, double r, double T, double sigma) {
    double d1 = (std::log(S / K) + (r + 0.5 * sigma * sigma) * T) / (sigma * std::sqrt(T));
    double d2 = d1 - sigma * std::sqrt(T);
    return -K * T * std::exp(-r * T) * norm_cdf(-d2);
}
