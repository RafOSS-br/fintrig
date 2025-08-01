#include "fintrig.h"
#include <types.h>

#define PRIMARY_BITMAP_SIZE 8

// static __inline int fintrig_is_valid(const struct fintrig_msg *msg) {
//     // if (msg->len == 0) {
//     //     return 0;
//     // }

//     // for (int i = 0; i < FINTRIG_MAX_FIELDS; i++) {
//     //     if (msg->offset[i] > msg->len) {
//     //         return 0;
//     //     }
//     // }

//     // return 1;
// }

/* Checks if a field is present */
__u8 fintrig_has_field(const struct fintrig_msg *msg, __u8 field) {
    if (!msg || !msg->spec || !msg->spec->fields)
        return 0;

    if (field < 1 || field > FINTRIG_MAX_FIELDS)
        return 0;

    if (field <= 64) {
        int byte_index = (field - 1) / 8;
        int bit_index = (field - 1) % 8;
        return (msg->bitmap[byte_index] & (0x80 >> bit_index)) ? 1 : 0;
    }

    /* Secondary bitmap (65–128) */
    if (!(has_secondary_bitmap(msg)))
        return 0;

    if (!msg->payload || msg->len < 16)
        return 0;

    const __u8 *secondary = msg->payload + 8;
    int byte_index = (field - 65) / 8;
    int bit_index = (field - 65) % 8;

    return (secondary[byte_index] & (0x80 >> bit_index)) ? 1 : 0;
}

/* Checks if a bucket is filled */
static __u8 fintrig_bucket_filled(const struct fintrig_msg *msg, int bucket_idx) {
    return msg->offset[bucket_idx] != 0;
}

static inline __u8 fintrig_is_digit(__u8 c) {
    return (c >= '0' && c <= '9') ? 1 : 0;
}

/* Gets the length of a field, safely checking boundaries */
static inline __u16 fintrig_get_field_len_safe(const struct fintrig_msg *msg,
                                               const __u8 *ptr,
                                               __u8 prefix) {
    if (!msg || !ptr) return 0;

    const __u8 *end = msg->payload + msg->len;

    switch (prefix) {
        case FINTRIG_PREFIX_LL:
            if (ptr + 2 > end) return 0;
            if (!fintrig_is_digit(ptr[0]) || !fintrig_is_digit(ptr[1])) return 0;
            return (ptr[0] - '0') * 10 + (ptr[1] - '0');

        case FINTRIG_PREFIX_LLL:
            if (ptr + 3 > end) return 0;
            if (!fintrig_is_digit(ptr[0]) ||
                !fintrig_is_digit(ptr[1]) ||
                !fintrig_is_digit(ptr[2])) return 0;
            return (ptr[0] - '0') * 100 +
                   (ptr[1] - '0') * 10 +
                   (ptr[2] - '0');

        case FINTRIG_PREFIX_FIXED:
        default:
            return 0;
    }
}

/* Builds the offset buckets */
static void fintrig_build_buckets(struct fintrig_msg *msg, int target_field) {
    if (!msg || !msg->spec || !msg->spec->fields)
        return;

    int total_fields = target_field;
    if (total_fields > FINTRIG_MAX_FIELDS)
        total_fields = FINTRIG_MAX_FIELDS;

    int fields_per_bucket = (total_fields + OFFSET_BUCKET_COUNT - 1) / OFFSET_BUCKET_COUNT;
    __u16 offset = 0;

    for (int bucket = 0; bucket < OFFSET_BUCKET_COUNT; bucket++) {
        if (fintrig_bucket_filled(msg, bucket))
            continue;

        int start_field = bucket * fields_per_bucket + 1;
        int end_field   = start_field + fields_per_bucket;
        if (start_field > total_fields)
            break;
        if (end_field > total_fields)
            end_field = total_fields;

        for (int field = start_field; field <= end_field; field++) {
            if (!fintrig_has_field(msg, field))
                continue;

            const struct fintrig_field_spec *spec = &msg->spec->fields[field];
            const __u8 *ptr = msg->payload + offset;
            __u16 field_len = 0;

            switch (spec->prefix) {
                case FINTRIG_PREFIX_FIXED:
                    field_len = spec->max_len;
                    break;

                case FINTRIG_PREFIX_LL:
                    field_len = fintrig_get_field_len_safe(msg, ptr, FINTRIG_PREFIX_LL);
                    if (field_len > 0) offset += 2;
                    break;

                case FINTRIG_PREFIX_LLL:
                    field_len = fintrig_get_field_len_safe(msg, ptr, FINTRIG_PREFIX_LLL);
                    if (field_len > 0) offset += 3;
                    break;
            }

            offset += field_len;
        }
        msg->offset[bucket] = offset;
    }
}


/* Gets pointer to the field, rebuilding buckets if necessary */
__u8* fintrig_get_field_ptr(struct fintrig_msg *msg, __u8 field) {
    if (!msg || !msg->spec || !msg->spec->fields)
        return NULL;

    if (field < 1 || field > FINTRIG_MAX_FIELDS)
        return NULL;

    /* Field index starts at 1 → adjust bucket index */
    int total_fields = field;
    if (total_fields > FINTRIG_MAX_FIELDS)
        total_fields = FINTRIG_MAX_FIELDS;
    int fields_per_bucket = (total_fields + OFFSET_BUCKET_COUNT - 1) / OFFSET_BUCKET_COUNT;
    int bucket_idx = (field - 1) / fields_per_bucket;

    if (!fintrig_bucket_filled(msg, bucket_idx)) {
        fintrig_build_buckets(msg, field);
    }

    __u16 offset = msg->offset[bucket_idx];
    int start_field = bucket_idx * fields_per_bucket + 1;
    int end_field = start_field + fields_per_bucket - 1;
    if (end_field > total_fields)
        end_field = total_fields;

    for (int f = start_field; f <= end_field && f < field; f++) {
        if (!fintrig_has_field(msg, f))
            continue;

        const struct fintrig_field_spec *spec = &msg->spec->fields[f];
        const __u8 *ptr = msg->payload + offset;
        __u16 field_len = 0;

        switch (spec->prefix) {
            case FINTRIG_PREFIX_FIXED:
                field_len = spec->max_len;
                break;
            case FINTRIG_PREFIX_LL:
                field_len = fintrig_get_field_len_safe(msg, ptr, FINTRIG_PREFIX_LL);
                if (field_len > 0) offset += 2;
                break;
            case FINTRIG_PREFIX_LLL:
                field_len = fintrig_get_field_len_safe(msg, ptr, FINTRIG_PREFIX_LLL);
                if (field_len > 0) offset += 3;
                break;
        }
        offset += field_len;
    }

    if (offset >= msg->len)
        return NULL; /* avoid out-of-bounds */

    return (__u8 *)(msg->payload + offset);
}


// static __inline int fintrig_get_field_len(const struct fintrig_msg *msg, int field) {

// }

