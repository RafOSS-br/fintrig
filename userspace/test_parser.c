#include <stdio.h>
#include <../core/fintrig.h>
#include <../core/spec_1987.h>
#include <linux/types.h>

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


int test_len() {
    printf("[parser length] running...\n");
    int fail = 0;

    ASSERT_EQ("test 1 - length == 0", fintrig_parse_iso_8583_1987(
        (__u8 *)"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 0, NULL), -1, fail);


    ASSERT_EQ("test 2 - fail in message lenght indicator", fintrig_parse_iso_8583_1987((__u8 *) "\x00\x0A\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 10, NULL), -1, fail);

    ASSERT_EQ("test 3 - pass message lenght indicator but fail in required fields", fintrig_parse_iso_8583_1987((__u8 *) "\x00\x0A\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 12, NULL), -1, fail);

    ASSERT_EQ("test 2 - length > 12", fintrig_parse_iso_8583_1987(
        (__u8 *)"\x00\x0C\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 14, NULL), 12, fail);


    return fail;
}


int test_parser() {
    printf("[parser] running...\n");
    int fail = 0;
    fail |= test_len();
    return fail;
}