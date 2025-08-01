#include <stdio.h>
#include <string.h>
#include <../core/fintrig.h>
#include <../core/spec_1987.h>
#include "test_util.h"

int test_bitmap_parsing() {
    printf("[bitmap parsing] running...\n");
    int fail = 0;

    #define MLI 16
    __u8 msg[MLI + 2] = {
        0x00, MLI, 0x12, 0x34, 0x56, 0x78,
        0xDE, 0xAD, 0xBE, 0xEF,
        0xCA, 0xFE, 0xBA, 0xBE,
        0x00, 0x00
    };

    struct fintrig_msg parsed;
    int res = fintrig_parse_iso_8583(msg, sizeof(msg), &iso_8583_1987_spec, &parsed);
    ASSERT_EQ("parse success", res, MLI, fail);

    const __u8 expected_bitmap[8] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xCA, 0xFE, 0xBA, 0xBE };
    ASSERT_MEMEQ("bitmap content", parsed.bitmap, expected_bitmap, 8, fail);

    return fail;
}

int test_secondary_bitmap_presence() {
    printf("[secondary bitmap presence] running...\n");
    int fail = 0;

    struct fintrig_msg msg = {0};
    msg.spec = &iso_8583_1987_spec;

    const __u8 bitmap_sec[8] = { 0x80, 0, 0, 0, 0, 0, 0, 0 };
    memcpy(msg.bitmap, bitmap_sec, 8);
    ASSERT_EQ("present", has_secondary_bitmap(&msg), 1, fail);

    const __u8 bitmap_no_sec[8] = { 0x00, 0, 0, 0, 0, 0, 0, 0 };
    memcpy(msg.bitmap, bitmap_no_sec, 8);
    ASSERT_EQ("absent", has_secondary_bitmap(&msg), 0, fail);

    ASSERT_EQ("msg NULL", has_secondary_bitmap(NULL), 0, fail);
    return fail;
}

int test_field_detection_primary() {
    printf("[field detection primary] running...\n");
    int fail = 0;

    struct fintrig_msg msg = {0};
    msg.spec = &iso_8583_1987_spec;

    const __u8 bitmap[8] = { 0xC0, 0, 0, 0, 0, 0, 0, 0 }; /* bits 1 and 2 active */
    memcpy(msg.bitmap, bitmap, 8);

    ASSERT_EQ("field 1", fintrig_has_field(&msg, 1), 1, fail);
    ASSERT_EQ("field 2", fintrig_has_field(&msg, 2), 1, fail);
    ASSERT_EQ("field 3 absent", fintrig_has_field(&msg, 3), 0, fail);

    return fail;
}

int test_field_detection_secondary() {
    printf("[field detection secondary] running...\n");
    int fail = 0;

    struct fintrig_msg msg = {0};
    msg.spec = &iso_8583_1987_spec;

    const __u8 primary[8] = { 0x80, 0, 0, 0, 0, 0, 0, 0 };
    memcpy(msg.bitmap, primary, 8);

    __u8 payload[16] = {0};
    memcpy(payload, primary, 8);
    payload[8] = 0x80; /* field 65 */
    msg.payload = payload;

    msg.len = sizeof(payload);

    ASSERT_EQ("secondary present", fintrig_has_field(&msg, 65), 1, fail);
    ASSERT_EQ("secondary absent 66", fintrig_has_field(&msg, 66), 0, fail);

    const __u8 no_sec[8] = { 0x00, 0, 0, 0, 0, 0, 0, 0 };
    memcpy(msg.bitmap, no_sec, 8);
    ASSERT_EQ("no secondary field 65", fintrig_has_field(&msg, 65), 0, fail);

    return fail;
}

int test_field_pointer_and_buckets() {
    printf("[field pointer and buckets] running...\n");
    int fail = 0;

    struct fintrig_msg msg = {0};
    msg.spec = &iso_8583_1987_spec;

    __u8 payload[256] = {0};
    msg.payload = payload;
    msg.len = sizeof(payload);

    /* Simulate present fields: 1–16 + field 50 */
    __u8 bitmap[8] = {0};
    for (int f = 1; f <= 16; f++) {
        int byte_index = (f - 1) / 8;
        int bit_index  = (f - 1) % 8;
        bitmap[byte_index] |= (0x80 >> bit_index);
    }
    /* Set bit for field 50 */
    int byte_index_50 = (50 - 1) / 8;
    int bit_index_50 = (50 - 1) % 8;
    bitmap[byte_index_50] |= (0x80 >> bit_index_50);

    memcpy(msg.bitmap, bitmap, sizeof(bitmap));

    /* Access field 10 */
    struct fintrig_field f = fintrig_get_field(&msg, 10);
    ASSERT_NEQ("field 10 pointer", f.ptr, NULL, fail);

    /* Access field 50 */
    f = fintrig_get_field(&msg, 50);
    ASSERT_NEQ("field 50 pointer", f.ptr, NULL, fail);

    return fail;
}
