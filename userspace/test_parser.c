#include <stdio.h>
#include <../core/fintrig.h>
#include <../core/spec_1987.h>

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

    ASSERT_EQ("null data", fintrig_parse_iso_8583(NULL, 10, &iso_8583_1987_spec, NULL), -1, fail);
    ASSERT_EQ("null spec", fintrig_parse_iso_8583((__u8 *)"\x00\x10...", 10, NULL, NULL), -1, fail);

    ASSERT_EQ("too short len", fintrig_parse_iso_8583((__u8 *)"\x00", 1, &iso_8583_1987_spec, NULL), -1, fail);
    ASSERT_EQ("len < MLI", fintrig_parse_iso_8583((__u8 *)"\x00\x0A\x12\x34", 4, &iso_8583_1987_spec, NULL), -1, fail);
    ASSERT_EQ("MLI < required fields", fintrig_parse_iso_8583(
        (__u8 *)"\x00\x05\x12\x34\x56\x78\x90", 7, &iso_8583_1987_spec, NULL), -1, fail);

    ASSERT_EQ("valid message", fintrig_parse_iso_8583(
        (__u8 *)"\x00\x10\x12\x34\x56\x78\x90\xAB\xCD\xEF\x00\x00\x00\x00\x00\x00\x00\x00", 18,
        &iso_8583_1987_spec, NULL), 16, fail);

    return fail;
}

int test_parser() {
    printf("[parser] running...\n");
    int fail = 0;
    fail |= test_len();
    return fail;
}
