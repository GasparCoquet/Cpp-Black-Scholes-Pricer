#include "../src/BlackScholes.h"
#include <cassert>
#include <iostream>
#include <cmath>

// Pricing Tests
void test_call_price() {
    double price = BlackScholes::call_price(100, 100, 0.05, 1.0, 0.2);
    // Reference value from known Black-Scholes calculators
    assert(std::abs(price - 10.4506) < 1e-3);
}

void test_put_price() {
    double price = BlackScholes::put_price(100, 100, 0.05, 1.0, 0.2);
    assert(std::abs(price - 5.5735) < 1e-3);
}

// Greeks Tests
void test_call_delta() {
    double delta = BlackScholes::call_delta(100, 100, 0.05, 1.0, 0.2);
    // ATM call delta ≈ 0.64 for these parameters
    assert(delta > 0.6 && delta < 0.7);
}

void test_put_delta() {
    double delta = BlackScholes::put_delta(100, 100, 0.05, 1.0, 0.2);
    // ATM put delta ≈ -0.36
    assert(delta > -0.4 && delta < -0.3);
}

void test_delta_parity() {
    // Call Delta - Put Delta = 1 (parity relationship)
    double call_delta = BlackScholes::call_delta(100, 100, 0.05, 1.0, 0.2);
    double put_delta = BlackScholes::put_delta(100, 100, 0.05, 1.0, 0.2);
    assert(std::abs(call_delta - put_delta - 1.0) < 1e-6);
}

void test_gamma() {
    double gamma = BlackScholes::gamma(100, 100, 0.05, 1.0, 0.2);
    // ATM gamma is positive
    assert(gamma > 0);
}

void test_vega() {
    double vega = BlackScholes::vega(100, 100, 0.05, 1.0, 0.2);
    // Vega is positive
    assert(vega > 0);
}

void test_call_theta() {
    double theta = BlackScholes::call_theta(100, 100, 0.05, 1.0, 0.2);
    // ATM call theta is typically negative (time decay)
    assert(theta < 0);
}

void test_put_theta() {
    double theta = BlackScholes::put_theta(100, 100, 0.05, 1.0, 0.2);
    // Put theta can be positive or negative depending on parameters
    // Just test it doesn't crash and returns a number
    assert(std::isfinite(theta));
}

void test_call_rho() {
    double rho = BlackScholes::call_rho(100, 100, 0.05, 1.0, 0.2);
    // Call rho is positive (calls benefit from higher rates)
    assert(rho > 0);
}

void test_put_rho() {
    double rho = BlackScholes::put_rho(100, 100, 0.05, 1.0, 0.2);
    // Put rho is negative (puts hurt from higher rates)
    assert(rho < 0);
}

// Edge case: Deep ITM
void test_deep_itm_call_delta() {
    double delta = BlackScholes::call_delta(150, 100, 0.05, 1.0, 0.2);
    // Deep ITM call delta ≈ 1
    assert(delta > 0.99);
}

// Edge case: Deep OTM
void test_deep_otm_call_delta() {
    double delta = BlackScholes::call_delta(50, 100, 0.05, 1.0, 0.2);
    // Deep OTM call delta ≈ 0
    assert(delta < 0.01);
}

// Edge case: Near expiry
void test_near_expiry_gamma() {
    double gamma_far = BlackScholes::gamma(100, 100, 0.05, 1.0, 0.2);
    double gamma_near = BlackScholes::gamma(100, 100, 0.05, 0.01, 0.2);
    // Gamma increases as expiry approaches
    assert(gamma_near > gamma_far);
}

int main() {
    // Pricing tests
    test_call_price();
    test_put_price();
    
    // Greeks tests
    test_call_delta();
    test_put_delta();
    test_delta_parity();
    test_gamma();
    test_vega();
    test_call_theta();
    test_put_theta();
    test_call_rho();
    test_put_rho();
    
    // Edge case tests
    test_deep_itm_call_delta();
    test_deep_otm_call_delta();
    test_near_expiry_gamma();
    
    std::cout << "All Black-Scholes tests passed!\n";
    return 0;
}
