#pragma once

#include <cmath>
#include <cstdio>

// ---------------------------------------------------------------------------
// A test assertion that survives NDEBUG.
//
// Why this exists:
//   The test suite used to be built out of bare `assert()`.  CI configures the
//   project with -DCMAKE_BUILD_TYPE=Release, which puts -DNDEBUG on the command
//   line, and <cassert> then expands every assert() to `((void)0)`.  The test
//   binary therefore evaluated *nothing*, printed "All tests passed!" and
//   exited 0 regardless of what the pricer returned: call_price() could have
//   returned 999999 and CI would still have been green.
//
//   CHECK/CHECK_NEAR are ordinary code.  They cannot be compiled away, they
//   report the offending file/line and the actual vs expected values, and they
//   make main() return a nonzero exit code so ctest and CI actually go red.
// ---------------------------------------------------------------------------

namespace test {
inline int checks = 0;    // total assertions evaluated
inline int failures = 0;  // assertions that failed
}  // namespace test

// Boolean condition.
#define CHECK(cond)                                                            \
    do {                                                                       \
        ++test::checks;                                                        \
        if (!(cond)) {                                                         \
            ++test::failures;                                                  \
            std::printf("FAIL %s:%d\n  CHECK(%s)\n", __FILE__, __LINE__,       \
                        #cond);                                                \
        }                                                                      \
    } while (0)

// |actual - expected| <= tol, with a readable failure message.
#define CHECK_NEAR(actual, expected, tol)                                      \
    do {                                                                       \
        ++test::checks;                                                        \
        const double check_a_ = (actual);                                      \
        const double check_e_ = (expected);                                    \
        const double check_t_ = (tol);                                         \
        const double check_d_ = std::fabs(check_a_ - check_e_);                \
        if (!(check_d_ <= check_t_)) {                                         \
            ++test::failures;                                                  \
            std::printf(                                                       \
                "FAIL %s:%d\n  CHECK_NEAR(%s, %s, %s)\n"                       \
                "    actual   = %.17g\n    expected = %.17g\n"                 \
                "    |diff|   = %.3g  (tolerance %.3g)\n",                     \
                __FILE__, __LINE__, #actual, #expected, #tol, check_a_,        \
                check_e_, check_d_, check_t_);                                 \
        }                                                                      \
    } while (0)

// |actual - expected| <= rel_tol * max(1, |expected|).  Used for the
// finite-difference Greek comparisons, where the magnitudes vary by orders of
// magnitude across the parameter grid.
#define CHECK_CLOSE_REL(actual, expected, rel_tol)                             \
    do {                                                                       \
        const double rel_e_ = (expected);                                      \
        const double rel_scale_ = std::fmax(1.0, std::fabs(rel_e_));           \
        CHECK_NEAR((actual), rel_e_, (rel_tol) * rel_scale_);                  \
    } while (0)

namespace test {

// Call this at the end of main().  Returns the process exit code.
inline int report(const char* suite) {
    if (failures > 0) {
        std::printf("\n%s: %d of %d checks FAILED\n", suite, failures, checks);
        return 1;
    }
    std::printf("\n%s: all %d checks passed\n", suite, checks);
    return 0;
}

}  // namespace test
