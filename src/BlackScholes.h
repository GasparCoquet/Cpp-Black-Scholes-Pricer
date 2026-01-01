#pragma once
#include <cmath>

class BlackScholes {
public:
    static double norm_cdf(double x);
    static double call_price(double S, double K, double r, double T, double sigma);
    static double put_price(double S, double K, double r, double T, double sigma);
        static double call_delta(double S, double K, double r, double T, double sigma);
        static double put_delta(double S, double K, double r, double T, double sigma);
        static double gamma(double S, double K, double r, double T, double sigma);
        static double vega(double S, double K, double r, double T, double sigma);
        static double call_theta(double S, double K, double r, double T, double sigma);
        static double put_theta(double S, double K, double r, double T, double sigma);
        static double call_rho(double S, double K, double r, double T, double sigma);
        static double put_rho(double S, double K, double r, double T, double sigma);
};
