#include "BlackScholes.h"
#include <vector>
#include <random>
#include <future>
#include <chrono>
#include <iostream>
#include <thread>

struct Option {
    double S, K, r, T, sigma;
    bool is_call;
};

std::vector<Option> generate_options(size_t n) {
    std::vector<Option> options(n);
    std::mt19937 gen(42);
    std::uniform_real_distribution<> dS(50, 150), dK(50, 150), dr(0.01, 0.1), dT(0.1, 2.0), dsigma(0.1, 0.5);
    std::bernoulli_distribution dcall(0.5);
    for (size_t i = 0; i < n; ++i) {
        options[i] = {dS(gen), dK(gen), dr(gen), dT(gen), dsigma(gen), dcall(gen)};
    }
    return options;
}

std::vector<double> price_single_thread(const std::vector<Option>& options) {
    std::vector<double> results(options.size());
    for (size_t i = 0; i < options.size(); ++i) {
        const auto& o = options[i];
        results[i] = o.is_call ? BlackScholes::call_price(o.S, o.K, o.r, o.T, o.sigma)
                               : BlackScholes::put_price(o.S, o.K, o.r, o.T, o.sigma);
    }
    return results;
}

std::vector<double> price_chunk(const std::vector<Option>& options, size_t start, size_t end) {
    std::vector<double> results(end - start);
    for (size_t i = start; i < end; ++i) {
        const auto& o = options[i];
        results[i - start] = o.is_call ? BlackScholes::call_price(o.S, o.K, o.r, o.T, o.sigma)
                                       : BlackScholes::put_price(o.S, o.K, o.r, o.T, o.sigma);
    }
    return results;
}

std::vector<double> price_multi_thread(const std::vector<Option>& options, int num_threads) {
    std::vector<std::future<std::vector<double>>> futures;
    const size_t n = options.size();
    const size_t chunk = n / num_threads;
    for (int t = 0; t < num_threads; ++t) {
        size_t start = t * chunk;
        size_t end = (t == num_threads - 1) ? n : (t + 1) * chunk;
        futures.push_back(std::async(std::launch::async, price_chunk, std::cref(options), start, end));
    }
    std::vector<double> results;
    for (auto& f : futures) {
        auto part = f.get();
        results.insert(results.end(), part.begin(), part.end());
    }
    return results;
}

int main() {
    constexpr size_t N = 1000000;
    const auto options = generate_options(N);
    int num_threads = std::thread::hardware_concurrency();
    if (num_threads < 2) num_threads = 2;

    auto t1 = std::chrono::high_resolution_clock::now();
    auto res1 = price_single_thread(options);
    auto t2 = std::chrono::high_resolution_clock::now();
    auto res2 = price_multi_thread(options, num_threads);
    auto t3 = std::chrono::high_resolution_clock::now();

    auto ms_single = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
    auto ms_multi = std::chrono::duration_cast<std::chrono::milliseconds>(t3 - t2).count();
    double speedup = ms_single / double(ms_multi);

    std::cout << "[Single-Threaded] Time: " << ms_single << "ms\n";
    std::cout << "[Multi-Threaded]  Time: " << ms_multi << "ms\n";
    std::cout << ">> Speedup: " << speedup << "x\n";
    return 0;
}
