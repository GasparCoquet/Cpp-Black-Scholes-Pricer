cmake_minimum_required(VERSION 3.10)
project(cpp-black-scholes-pricer)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if (MSVC)
	add_compile_options("$<$<CONFIG:Release>:/O2>" "$<$<CONFIG:Release>:/arch:AVX2>")
else()
	add_compile_options("$<$<CONFIG:Release>:-O3>" "$<$<CONFIG:Release>:-march=native>")
endif()

# C++ executables
add_executable(cpp-black-scholes-pricer src/main.cpp src/BlackScholes.cpp)
add_executable(benchmark src/main_benchmark.cpp src/BlackScholes.cpp)

# Python bindings (optional - only if pybind11 is found)
find_package(pybind11 QUIET)
if(pybind11_FOUND)
    pybind11_add_module(black_scholes src/bindings.cpp src/BlackScholes.cpp)
    message(STATUS "Pybind11 found - Python module will be built")
else()
    message(STATUS "Pybind11 not found - skipping Python module (install with: pip install pybind11)")
endif()

# Testing
enable_testing()
add_executable(test_greeks tests/test_greeks.cpp src/BlackScholes.cpp)
add_test(NAME GreeksTest COMMAND test_greeks)
