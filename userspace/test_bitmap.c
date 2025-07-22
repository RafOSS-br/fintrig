#include <stdio.h>
#include <string.h>
#include <../core/fintrig.h>
#include <../core/spec_1987.h>
#include "test_util.h"

int test_bitmap() {
    printf("[bitmap] running...\n");
    int fail = 0;

    #define MLI 16  // payload = 16 bytes
    __u8 msg[MLI + 2] = {
        0x00, MLI,                         // MLI = 16
        0x12, 0x34, 0x56, 0x78,            // MTI (4 bytes)
        0xDE, 0xAD, 0xBE, 0xEF,            // bitmap[0..3]
        0xCA, 0xFE, 0xBA, 0xBE,            // bitmap[4..7]
        0x00, 0x00                         // dummy payload
    };

    struct fintrig_msg parsed;
    int res = fintrig_parse_iso_8583(msg, sizeof(msg), &iso_8583_1987_spec, &parsed);
    ASSERT_EQ("bitmap parse success", res, MLI, fail);

    const __u8 expected_bitmap[8] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xCA, 0xFE, 0xBA, 0xBE };
    ASSERT_MEMEQ("bitmap content match", parsed.bitmap, expected_bitmap, 8, fail);

    return fail;
}