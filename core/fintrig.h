#pragma once

#include <types.h>

#define FINTRIG_MAX_FIELDS 64
#define OFFSET_BUCKET_SIZE 5
#define OFFSET_BUCKET_COUNT ((FINTRIG_MAX_FIELDS + OFFSET_BUCKET_SIZE) / OFFSET_BUCKET_SIZE)


/* prefix length: 0 = fixed, 2 = LL, 3 = LLL */
enum fintrig_prefix {
    FINTRIG_PREFIX_FIXED = 0,
    FINTRIG_PREFIX_LL    = 2,
    FINTRIG_PREFIX_LLL   = 3,
};

/* per-field specification */
struct fintrig_field_spec {
    __u8  prefix;   /* length of length field (0,2,3) */
    __u16 max_len;  /* maximum data length */
    __u8 required; /* 0 = optional, 1 = required */
};

/* message-level specification */
struct fintrig_message_spec {
    const struct fintrig_field_spec *fields;  /* pointer to array of FINTRIG_MAX_FIELDS+1 specs */
};

/* parsed message */
struct fintrig_msg {
    const struct fintrig_message_spec *spec;  /* which spec to use */
    __u8  bitmap[8];                       /* primary bitmap */
    const __u8 *payload;                   /* raw ISO8583 data */
    __u16 len;                              /* total payload length */
    __u16 offset[OFFSET_BUCKET_COUNT];  /* offset of each bucket of fields (0 if all absent in bucket) */
};

static __inline int fintrig_is_valid(const struct fintrig_msg *msg);

static __inline int fintrig_has_field(const struct fintrig_msg *msg, int field);

static __inline const __u8* fintrig_get_field_ptr(const struct fintrig_msg *msg, int field);

static __inline int fintrig_get_field_len(const struct fintrig_msg *msg, int field);
