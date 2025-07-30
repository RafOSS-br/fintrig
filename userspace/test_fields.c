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
    const __u8 bitmap[8] = { 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
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
