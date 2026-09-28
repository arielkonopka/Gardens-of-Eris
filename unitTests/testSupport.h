#ifndef GOE_TEST_SUPPORT_H
#define GOE_TEST_SUPPORT_H

#include <gtest/gtest.h>
#include <stdexcept>

/// ASSERT_* only works in functions returning void; helpers that return a value use this instead.
/// It records the failure and throws, and GoogleTest reports the exception as the test failing.
#define REQUIRE_IN_HELPER(cond)                                                                    \
    do {                                                                                           \
        if (!(cond)) {                                                                             \
            ADD_FAILURE() << "Required: " #cond;                                                   \
            throw std::runtime_error("required check failed: " #cond);                             \
        }                                                                                          \
    } while (0)

#endif // GOE_TEST_SUPPORT_H
