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


int test_has_secondary_bitmap() {
    printf("[has_secondary_bitmap] running...\n");
    int fail = 0;

    struct fintrig_msg msg = {0};
    msg.spec = &iso_8583_1987_spec;

    /* Primary bitmap with bit 1 set → secondary bitmap present */
    const __u8 bitmap_with_secondary[8] = { 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
    memcpy(msg.bitmap, bitmap_with_secondary, sizeof(bitmap_with_secondary));
    ASSERT_EQ("secondary bitmap present", has_secondary_bitmap(&msg), 1, fail);

    /* Primary bitmap without bit 1 → without secondary */
    const __u8 bitmap_no_secondary[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
    memcpy(msg.bitmap, bitmap_no_secondary, sizeof(bitmap_no_secondary));
    ASSERT_EQ("secondary bitmap absent", has_secondary_bitmap(&msg), 0, fail);

    /* Message with NULL pointer */
    ASSERT_EQ("msg NULL", has_secondary_bitmap(NULL), 0, fail);

    return fail;
}