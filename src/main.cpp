#include <iomanip>
#include <iostream>

#include "BlackScholes.h"

int main() {
    const double S = 100;     // Spot price
    const double K = 100;     // Strike price
    const double r = 0.05;    // Risk-free rate
    const double T = 1.0;     // Time to maturity (years)
    const double sigma = 0.2; // Volatility

    const double call = BlackScholes::call_price(S, K, r, T, sigma);
    const double put = BlackScholes::put_price(S, K, r, T, sigma);

    std::cout << "Call Price: " << call << std::endl;
    std::cout << "Put Price:  " << put << std::endl;
    std::cout << "Call Delta: " << BlackScholes::call_delta(S, K, r, T, sigma) << std::endl;
    std::cout << "Put  Delta: " << BlackScholes::put_delta(S, K, r, T, sigma) << std::endl;
    std::cout << "Gamma:      " << BlackScholes::gamma(S, K, r, T, sigma) << std::endl;
    std::cout << "Vega:       " << BlackScholes::vega(S, K, r, T, sigma) << std::endl;
    std::cout << "Call Theta: " << BlackScholes::call_theta(S, K, r, T, sigma) << std::endl;
    std::cout << "Put  Theta: " << BlackScholes::put_theta(S, K, r, T, sigma) << std::endl;
    std::cout << "Call Rho:   " << BlackScholes::call_rho(S, K, r, T, sigma) << std::endl;
    std::cout << "Put  Rho:   " << BlackScholes::put_rho(S, K, r, T, sigma) << std::endl;

    // Implied volatility: invert the price back to the vol that produced it.
    const double iv = BlackScholes::call_implied_vol(call, S, K, r, T);
    std::cout << "\nImplied vol from the call price above:\n";
    std::cout << "  input sigma:  " << std::setprecision(17) << sigma << "\n";
    std::cout << "  recovered:    " << iv << "\n";
    std::cout << "  |error|:      " << std::abs(iv - sigma) << std::setprecision(6) << std::endl;

    // A dividend-paying underlying (q = 3%).
    const double q = 0.03;
    std::cout << "\nWith a 3% continuous dividend yield:\n";
    std::cout << "  Call Price: " << BlackScholes::call_price(S, K, r, T, sigma, q) << "\n";
    std::cout << "  Put Price:  " << BlackScholes::put_price(S, K, r, T, sigma, q) << "\n";
    std::cout << "  Call Delta: " << BlackScholes::call_delta(S, K, r, T, sigma, q) << std::endl;

    // Degenerate inputs return their limiting value, not a silent NaN.
    std::cout << "\nAt expiry (T = 0), an option is worth its intrinsic value:\n";
    std::cout << "  call(S=120, K=100, T=0): " << BlackScholes::call_price(120, 100, r, 0.0, sigma) << "\n";
    std::cout << "  put (S=120, K=100, T=0): " << BlackScholes::put_price(120, 100, r, 0.0, sigma) << std::endl;

    return 0;
}
