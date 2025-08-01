#pragma once

#include <types.h>

#define FINTRIG_MAX_FIELDS 128
#define OFFSET_BUCKET_SIZE 5
#define OFFSET_BUCKET_COUNT ((FINTRIG_MAX_FIELDS + OFFSET_BUCKET_SIZE) / OFFSET_BUCKET_SIZE)

/* prefix length: 0 = fixed, 2 = LL, 3 = LLL, 4 = TLV */
enum fintrig_prefix {
    FINTRIG_PREFIX_FIXED = 0,
    FINTRIG_PREFIX_LL    = 2,
    FINTRIG_PREFIX_LLL   = 3,
    FINTRIG_PREFIX_TLV   = 4,
};

/* per-field specification */
struct fintrig_field_spec {
    __u8  prefix;   /* length of length field (0,2,3,4) */
    __u16 max_len;  /* maximum data length */
    __u8 required;  /* 0 = optional, 1 = required */
};

/* message-level specification */
struct fintrig_message_spec {
    const struct fintrig_field_spec *fields;  /* pointer to array of FINTRIG_MAX_FIELDS+1 specs */
};

/* parsed message */
struct fintrig_msg {
    const struct fintrig_message_spec *spec;  /* which spec to use */
    __u8  bitmap[8];                          /* primary bitmap */
    const __u8 *payload;                      /* raw ISO8583 data */
    __u16 len;                                /* total payload length */
    __u16 offset[OFFSET_BUCKET_COUNT];        /* offset of each bucket of fields (0 if all absent in bucket) */
    __u16 built_offset;                       /* byte offset after last built bucket */
    __u8  built_bucket;                       /* index of last built bucket */
    __u8  built_field;                        /* last field fully processed */
};

/* Create a new empty message */
static inline struct fintrig_msg fintrig_msg_new(void) {
    struct fintrig_msg msg;

    msg.spec = NULL;
    msg.payload = NULL;
    msg.len = 0;
    msg.built_offset = 0;
    msg.built_bucket = 0;
    msg.built_field = 0;

    for (int i = 0; i < OFFSET_BUCKET_COUNT; i++) {
        msg.offset[i] = 0xFFFF; /* bucket not filled */
    }

    for (int i = 0; i < 8; i++) {
        msg.bitmap[i] = 0;
    }

    return msg;
}

/* Generic field object */
struct fintrig_field {
    const __u8 *ptr;     /* pointer to field data */
    __u16 len;           /* length of field data */
    __u8 is_tlv;         /* 1 if TLV field */
};

/* TLV structure */
struct fintrig_tlv {
    __u8 tag;
    __u8 len;
    const __u8 *value;
};

/* API */
__u8 fintrig_has_field(const struct fintrig_msg *msg, __u8 field);
struct fintrig_field fintrig_get_field(const struct fintrig_msg *msg, __u8 field);

static inline __u8 has_secondary_bitmap(const struct fintrig_msg *msg) {
    return fintrig_has_field(msg, 1);
}

/* Field helpers */
static inline __u8 fintrig_field_is_tlv(const struct fintrig_field *f) {
    return f && f->is_tlv;
}

static inline const __u8* fintrig_field_value(const struct fintrig_field *f, __u16 *len) {
    if (!f) return NULL;
    if (len) *len = f->len;
    return f->ptr;
}

/* TLV helpers */
__u8 fintrig_tlv_next(const __u8 *ptr, __u16 remaining, struct fintrig_tlv *out);
