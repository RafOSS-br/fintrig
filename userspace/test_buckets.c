#include <stdio.h>
#include <string.h>
#include <../core/fintrig.h>
#include <../core/spec_1987.h>
#include "test_util.h"

/* Helper: creates dummy message with all fields present */
static void prepare_msg_with_fields(struct fintrig_msg *msg, int total_fields) {
    memset(msg, 0, sizeof(*msg));
    msg->spec = &iso_8583_1987_spec;
    static __u8 payload[512]; /* fake buffer */
    msg->payload = payload;
    msg->len = sizeof(payload);

    /* Activate bits up to total_fields */
    memset(msg->bitmap, 0x00, sizeof(msg->bitmap));
    for (int f = 1; f <= total_fields && f <= 64; f++) {
        int byte_index = (f - 1) / 8;
        int bit_index  = (f - 1) % 8;
        msg->bitmap[byte_index] |= (0x80 >> bit_index);
    }
}

int test_bucket_initial_build() {
    printf("[bucket initial build] running...\n");
    int fail = 0;
    struct fintrig_msg msg;

    prepare_msg_with_fields(&msg, 20);

    /* No bucket filled */
    for (int i = 0; i < OFFSET_BUCKET_COUNT; i++) {
        ASSERT_EQ("bucket initially zero", msg.offset[i], 0, fail);
    }

    /* Call to access field 10 → should build up to there */
    __u8 *ptr = fintrig_get_field_ptr(&msg, 10);
    ASSERT_NEQ("ptr not NULL", ptr, NULL, fail);

    int filled = 0;
    for (int i = 0; i < OFFSET_BUCKET_COUNT; i++) {
        if (msg.offset[i] != 0)
            filled++;
    }
    ASSERT_NEQ("some buckets filled", filled, 0, fail);

    return fail;
}

int test_bucket_incremental_build() {
    printf("[bucket incremental build] running...\n");
    int fail = 0;
    struct fintrig_msg msg;

    prepare_msg_with_fields(&msg, 50);

    /* Access field 30 */
    fintrig_get_field_ptr(&msg, 30);

    int filled_after_30 = 0;
    for (int i = 0; i < OFFSET_BUCKET_COUNT; i++) {
        if (msg.offset[i] != 0)
            filled_after_30++;
    }
    ASSERT_NEQ("buckets built for field 30", filled_after_30, 0, fail);

    /* Now access field 50 → should expand */
    fintrig_get_field_ptr(&msg, 50);

    int filled_after_50 = 0;
    for (int i = 0; i < OFFSET_BUCKET_COUNT; i++) {
        if (msg.offset[i] != 0)
            filled_after_50++;
    }
    ASSERT_NEQ("buckets expanded for field 50", filled_after_50 >= filled_after_30, 0, fail);

    return fail;
}

int test_bucket_reuse() {
    printf("[bucket reuse] running...\n");
    int fail = 0;
    struct fintrig_msg msg;

    prepare_msg_with_fields(&msg, 40);

    /* First access builds buckets */
    fintrig_get_field_ptr(&msg, 20);
    int filled_before = 0;
    for (int i = 0; i < OFFSET_BUCKET_COUNT; i++) {
        if (msg.offset[i] != 0)
            filled_before++;
    }

    /* Second access should not rebuild everything (same region) */
    fintrig_get_field_ptr(&msg, 25);
    int filled_after = 0;
    for (int i = 0; i < OFFSET_BUCKET_COUNT; i++) {
        if (msg.offset[i] != 0)
            filled_after++;
    }

    ASSERT_EQ("buckets unchanged after reuse", filled_before, filled_after, fail);

    return fail;
}
