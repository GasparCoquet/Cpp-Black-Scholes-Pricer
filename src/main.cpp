#include <iostream>
#include "BlackScholes.h"

int main() {
    double S = 100;    // Spot price
    double K = 100;    // Strike price
    double r = 0.05;   // Risk-free rate
    double T = 1.0;    // Time to maturity (years)
    double sigma = 0.2;// Volatility

    double call = BlackScholes::call_price(S, K, r, T, sigma);
    double put = BlackScholes::put_price(S, K, r, T, sigma);

    // Greeks
    double call_delta = BlackScholes::call_delta(S, K, r, T, sigma);
    double put_delta  = BlackScholes::put_delta(S, K, r, T, sigma);
    double gamma      = BlackScholes::gamma(S, K, r, T, sigma);
    double vega       = BlackScholes::vega(S, K, r, T, sigma);
    double call_theta = BlackScholes::call_theta(S, K, r, T, sigma);
    double put_theta  = BlackScholes::put_theta(S, K, r, T, sigma);
    double call_rho   = BlackScholes::call_rho(S, K, r, T, sigma);
    double put_rho    = BlackScholes::put_rho(S, K, r, T, sigma);

    std::cout << "Call Price: " << call << std::endl;
    std::cout << "Put Price:  " << put << std::endl;
    std::cout << "Call Delta: " << call_delta << std::endl;
    std::cout << "Put  Delta: " << put_delta << std::endl;
    std::cout << "Gamma:      " << gamma << std::endl;
    std::cout << "Vega:       " << vega << std::endl;
    std::cout << "Call Theta: " << call_theta << std::endl;
    std::cout << "Put  Theta: " << put_theta << std::endl;
    std::cout << "Call Rho:   " << call_rho << std::endl;
    std::cout << "Put  Rho:   " << put_rho << std::endl;
    return 0;
}
