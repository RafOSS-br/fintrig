#include <stdio.h>
#include <string.h>
#include <../core/fintrig.h>
#include <../core/spec_1987.h>
#include "test_util.h"

int test_field_object_basic() {
    printf("[field object basic] running...\n");
    int fail = 0;

    struct fintrig_msg msg = {0};
    msg.spec = &iso_8583_1987_spec;

    __u8 payload[32] = { '4','2','1','2','3','4','5','6' };
    msg.payload = payload;
    msg.len = sizeof(payload);

    /* Simulate field present */
    const __u8 bitmap[8] = { 0x40, 0, 0, 0, 0, 0, 0, 0 }; /* field 2 active */
    memcpy(msg.bitmap, bitmap, sizeof(bitmap));

    struct fintrig_field f = fintrig_get_field(&msg, 2);
    ASSERT_NEQ("field object pointer", f.ptr, NULL, fail);
    ASSERT_EQ("field object len > 0", f.len > 0, 1, fail);
    ASSERT_EQ("field object is not TLV", fintrig_field_is_tlv(&f), 0, fail);

    return fail;
}

int test_field_object_tlv_iteration() {
    printf("[field object TLV iteration] running...\n");
    int fail = 0;

    struct fintrig_msg msg = {0};

    /* Make a mutable copy of the spec */
    struct fintrig_field_spec fields_copy[FINTRIG_MAX_FIELDS + 1];
    memcpy(fields_copy, iso_8583_1987_fields, sizeof(fields_copy));
    fields_copy[3].prefix = FINTRIG_PREFIX_TLV;
    fields_copy[3].max_len = 8; // TLV field max length

    struct fintrig_message_spec spec_copy = {
        .fields = fields_copy
    };
    msg.spec = &spec_copy;

    /* TLV payload: tag=0x01 len=2 val="AB", tag=0x02 len=1 val="Z" */
    __u8 payload[8] = { 0x01, 0x02, 'A', 'B', 0x02, 0x01, 'Z', 0x00 };
    msg.payload = payload;
    msg.len = sizeof(payload);

    /* Simulate TLV field */
    const __u8 bitmap[8] = { 0x20, 0, 0, 0, 0, 0, 0, 0 }; /* field 3 active */
    memcpy(msg.bitmap, bitmap, sizeof(bitmap));

    struct fintrig_field f = fintrig_get_field(&msg, 3);

    const __u8 *p = f.ptr;
    __u16 rem = f.len;
    struct fintrig_tlv tlv;
    int count = 0;

    while (fintrig_tlv_next(p, rem, &tlv)) {
        count++;
        p += 2 + tlv.len;
        rem -= 2 + tlv.len;
        printf("TLV tag=0x%02X len=%d value='%.*s'\n", tlv.tag, tlv.len, tlv.len, tlv.value);
    }

    ASSERT_EQ("parsed 2 TLVs", count, 2, fail);

    return fail;
}