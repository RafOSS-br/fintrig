#include <stdio.h>
#include <string.h>
#include <../core/fintrig.h>
#include <../core/spec_1987.h>
#include "test_util.h"

int test_has_field() {
    printf("[has_field] running...\n");
    int fail = 0;

    struct fintrig_msg msg = {0};
    msg.spec = &iso_8583_1987_spec;
    const __u8 bitmap[8] = { 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
    memcpy(msg.bitmap, bitmap, sizeof(bitmap));

    ASSERT_EQ("field 1 present", fintrig_has_field(&msg, 1), 1, fail);
    ASSERT_EQ("field 2 absent", fintrig_has_field(&msg, 2), 0, fail);
    ASSERT_EQ("invalid field 0", fintrig_has_field(&msg, 0), 0, fail);
    ASSERT_EQ("invalid field > max", fintrig_has_field(&msg, FINTRIG_MAX_FIELDS + 1), 0, fail);
    ASSERT_EQ("invalid field < 1", fintrig_has_field(&msg, 0xFF), 0, fail);
    ASSERT_EQ("invalid msg", fintrig_has_field(NULL, 1), 0, fail);
    ASSERT_EQ("invalid msg spec", fintrig_has_field(&(struct fintrig_msg){0}, 1), 0, fail);
    ASSERT_EQ("invalid msg spec fields", fintrig_has_field(&(struct fintrig_msg){.spec = &(struct fintrig_message_spec){0}}, 1), 0, fail);
    ASSERT_EQ("invalid msg spec fields array", fintrig_has_field(&(struct fintrig_msg){.spec = &(struct fintrig_message_spec){.fields = NULL}}, 1), 0, fail);

    return fail;
}

int test_has_field_with_secondary() {
    printf("[has_field_with_secondary] running...\n");
    int fail = 0;

    struct fintrig_msg msg = {0};
    msg.spec = &iso_8583_1987_spec;

    /* Primary with bit 1 set → secondary exists */
    const __u8 primary_bitmap[8] = { 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
    memcpy(msg.bitmap, primary_bitmap, sizeof(primary_bitmap));

    /* Secondary bitmap with field 65 present */
    __u8 payload[16] = {0};
    memcpy(payload, primary_bitmap, 8);      // Primary in payload
    payload[8] = 0x80;                       // Field 65 present (MSB bit)
    msg.payload = payload;
    msg.len = sizeof(payload);

    ASSERT_EQ("secondary bitmap detected", fintrig_has_field(&msg, 1), 1, fail);
    ASSERT_EQ("field 65 present", fintrig_has_field(&msg, 65), 1, fail);
    ASSERT_EQ("field 66 absent", fintrig_has_field(&msg, 66), 0, fail);

    /* Without secondary → field 65 must be absent */
    const __u8 primary_no_secondary[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
    memcpy(msg.bitmap, primary_no_secondary, sizeof(primary_no_secondary));
    ASSERT_EQ("field 65 without secondary", fintrig_has_field(&msg, 65), 0, fail);

    return fail;
}
