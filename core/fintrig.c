#include "fintrig.h"
#include <types.h>

#define PRIMARY_BITMAP_SIZE 8

/* --------------------------------------------------------------------------
 * Helpers: bitmap and field length
 * -------------------------------------------------------------------------- */

/* Checks if a field exists in bitmap (primary or secondary) */
__u8 fintrig_has_field(const struct fintrig_msg *msg, __u8 field) {
    if (!msg || !msg->spec || !msg->spec->fields)
        return 0;

    if (field < 1 || field > FINTRIG_MAX_FIELDS)
        return 0;

    /* Primary bitmap: fields 1–64 */
    if (field <= 64) {
        int byte_index = (field - 1) / 8;
        int bit_index = (field - 1) % 8;
        return (msg->bitmap[byte_index] & (0x80 >> bit_index)) ? 1 : 0;
    }

    /* Secondary bitmap required for fields 65–128 */
    if (!has_secondary_bitmap(msg))
        return 0;

    if (!msg->payload || msg->len < 16)
        return 0;

    const __u8 *secondary = msg->payload + 8;
    int byte_index = (field - 65) / 8;
    int bit_index = (field - 65) % 8;

    return (secondary[byte_index] & (0x80 >> bit_index)) ? 1 : 0;
}

/* Returns true if char is numeric */
static inline __u8 fintrig_is_digit(__u8 c) {
    return (c >= '0' && c <= '9') ? 1 : 0;
}

/* Safe parser for field length based on prefix */
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

/* --------------------------------------------------------------------------
 * Helpers: bucket management
 * -------------------------------------------------------------------------- */

/* Returns true if a bucket offset has been computed */
static __u8 fintrig_bucket_filled(const struct fintrig_msg *msg, int bucket_idx) {
    return msg->offset[bucket_idx] != UINT16_MAX;
}

/* Build offset buckets incrementally up to a target field */
static void fintrig_build_buckets(struct fintrig_msg *msg, int target_field) {
    if (!msg || !msg->spec || !msg->spec->fields)
        return;

    int total_fields = target_field > FINTRIG_MAX_FIELDS ? FINTRIG_MAX_FIELDS : target_field;
    int fields_per_bucket = (FINTRIG_MAX_FIELDS + OFFSET_BUCKET_COUNT - 1) / OFFSET_BUCKET_COUNT;

    __u16 offset = msg->built_offset;
    int bucket_start = msg->built_bucket;

    for (int bucket = bucket_start; bucket < OFFSET_BUCKET_COUNT; bucket++) {

        int start_field = bucket * fields_per_bucket + 1;
        int end_field   = start_field + fields_per_bucket - 1;
        if (end_field > FINTRIG_MAX_FIELDS)
            end_field = FINTRIG_MAX_FIELDS;

        msg->offset[bucket] = offset;

        for (int field = start_field; field <= end_field && field <= total_fields; field++) {
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
                case FINTRIG_PREFIX_TLV:
                    field_len = spec->max_len;
                    break;
            }

            offset += field_len;
            msg->built_field = field;
        }

        msg->built_bucket = bucket + 1;
        msg->built_offset = offset;

        if (end_field >= total_fields)
            break;
    }
}

/* --------------------------------------------------------------------------
 * Field access API
 * -------------------------------------------------------------------------- */

/* Get a fully described field object */
struct fintrig_field fintrig_get_field(const struct fintrig_msg *msg, __u8 field) {
    struct fintrig_field out = { .ptr = NULL, .len = 0, .is_tlv = 0 };

    if (!msg || !msg->spec || !msg->spec->fields)
        return out;
    if (field < 1 || field > FINTRIG_MAX_FIELDS)
        return out;
    if (!fintrig_has_field(msg, field))
        return out;

    /* Bucket calculation */
    int fields_per_bucket = (FINTRIG_MAX_FIELDS + OFFSET_BUCKET_COUNT - 1) / OFFSET_BUCKET_COUNT;
    int bucket_idx = (field - 1) / fields_per_bucket;

    /* Build missing buckets */
    if (!fintrig_bucket_filled(msg, bucket_idx)) {
        fintrig_build_buckets((struct fintrig_msg *)msg, field);
    }

    __u16 offset = msg->offset[bucket_idx];
    int start_field = bucket_idx * fields_per_bucket + 1;
    int end_field = start_field + fields_per_bucket - 1;
    if (end_field > FINTRIG_MAX_FIELDS)
        end_field = FINTRIG_MAX_FIELDS;

    /* Skip previous fields to reach target */
    for (int f = start_field; f < field && f <= end_field; f++) {
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
            case FINTRIG_PREFIX_TLV:
                field_len = spec->max_len;
                break;
        }
        offset += field_len;
    }

    if (offset >= msg->len)
        return out;

    /* Save the start of target field before adjusting for prefix */
    __u16 field_start_offset = offset;

    const struct fintrig_field_spec *target_spec = &msg->spec->fields[field];
    __u16 target_len = 0;

    switch (target_spec->prefix) {
        case FINTRIG_PREFIX_FIXED:
            target_len = target_spec->max_len;
            break;
        case FINTRIG_PREFIX_LL:
            target_len = fintrig_get_field_len_safe(msg, msg->payload + offset, FINTRIG_PREFIX_LL);
            offset += 2;
            break;
        case FINTRIG_PREFIX_LLL:
            target_len = fintrig_get_field_len_safe(msg, msg->payload + offset, FINTRIG_PREFIX_LLL);
            offset += 3;
            break;
        case FINTRIG_PREFIX_TLV: {
            /* Calculate real TLV length */
            const __u8 *ptr_tlv = msg->payload + field_start_offset;
            __u16 remaining = target_spec->max_len;
            __u16 consumed = 0;

            while (remaining >= 2) {
                __u8 tlv_len = ptr_tlv[1];
                if (remaining < (2 + tlv_len))
                    break;
                consumed += 2 + tlv_len;
                ptr_tlv += 2 + tlv_len;
                remaining -= 2 + tlv_len;
            }

            target_len = consumed;
            out.is_tlv = 1;
            break;
        }
    }

    out.ptr = msg->payload + field_start_offset;
    out.len = target_len;

    return out;
}

/* --------------------------------------------------------------------------
 * TLV utilities
 * -------------------------------------------------------------------------- */

/* Get next TLV from a buffer */
__u8 fintrig_tlv_next(const __u8 *ptr, __u16 remaining, struct fintrig_tlv *out) {
    if (remaining < 2) return 0;

    __u8 tag = ptr[0];
    __u8 len = ptr[1];
    if (remaining < (2 + len)) return 0;

    out->tag = tag;
    out->len = len;
    out->value = ptr + 2;

    return 1;
}
