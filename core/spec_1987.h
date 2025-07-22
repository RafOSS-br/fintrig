#pragma once

#include "fintrig.h"
#include <types.h>

/* ISO 8583 v1987 ASCII spec (partial) */
static const struct fintrig_field_spec iso_8583_1987_fields[FINTRIG_MAX_FIELDS + 1] = {
    [0]  = { .prefix = FINTRIG_PREFIX_FIXED, .max_len = 4, .required = 1  },  /* MTI */
    [1]  = { .prefix = FINTRIG_PREFIX_FIXED, .max_len = 8, .required = 1  },  /* Bitmap */
    [2]  = { .prefix = FINTRIG_PREFIX_LL,    .max_len = 19, .required = 0  },  /* PAN */
    [3]  = { .prefix = FINTRIG_PREFIX_FIXED, .max_len = 6,  .required = 0  },  /* Processing Code */
    [4]  = { .prefix = FINTRIG_PREFIX_FIXED, .max_len = 12, .required = 0  },  /* Amount */
    [35] = { .prefix = FINTRIG_PREFIX_LL,    .max_len = 37, .required = 0  },  /* Track 2 */
    [36] = { .prefix = FINTRIG_PREFIX_LLL,   .max_len = 104, .required = 0 },  /* Track 3 */
    [37] = { .prefix = FINTRIG_PREFIX_FIXED, .max_len = 12, .required = 0  },  /* Retrieval Ref */
    [64] = { .prefix = FINTRIG_PREFIX_FIXED, .max_len = 8,  .required = 0  },  /* MAC */
};

static const struct fintrig_message_spec iso_8583_1987_spec = {
    .fields = iso_8583_1987_fields,
};

int fintrig_parse_iso_8583(const __u8 *data, __u16 len,
                           const struct fintrig_message_spec *spec,
                           struct fintrig_msg *out);