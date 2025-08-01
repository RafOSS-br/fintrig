#pragma once
#include <stdio.h>
#include <string.h>

#define ASSERT_EQ(label, expr, expected, fail_var)                   \
    do {                                                             \
        int result = (expr);                                         \
        if (result != (expected)) {                                  \
            printf("[%s] FAIL (expected %d, got %d)\n", label, expected, result); \
            (fail_var) = 1;                                          \
        } else {                                                     \
            printf("[%s] PASS\n", label);                            \
        }                                                            \
    } while (0)

#define ASSERT_MEMEQ(label, a, b, len, fail_var)                     \
    do {                                                             \
        if (memcmp((a), (b), (len)) != 0) {                          \
            printf("[%s] FAIL (memcmp mismatch)\n", label);          \
            (fail_var) = 1;                                          \
        } else {                                                     \
            printf("[%s] PASS\n", label);                            \
        }                                                            \
    } while (0)

#define ASSERT_NEQ(label, expr, not_expected, fail_var)              \
    do {                                                             \
        int result = (expr != (not_expected));                        \
        if (!result) {                                               \
            printf("[%s] FAIL (value equals unexpected %ld)\n", label, (long)(not_expected)); \
            (fail_var) = 1;                                          \
        } else {                                                     \
            printf("[%s] PASS\n", label);                            \
        }                                                            \
    } while (0)
