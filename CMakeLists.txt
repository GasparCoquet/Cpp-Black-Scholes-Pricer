cmake_minimum_required(VERSION 3.10)
project(cpp-black-scholes-pricer)
set(CMAKE_CXX_STANDARD 17)

add_executable(cpp-black-scholes-pricer src/main.cpp src/BlackScholes.cpp)
add_executable(benchmark src/main_benchmark.cpp src/BlackScholes.cpp)

enable_testing()
add_executable(test_greeks tests/test_greeks.cpp src/BlackScholes.cpp)
add_test(NAME GreeksTest COMMAND test_greeks)
