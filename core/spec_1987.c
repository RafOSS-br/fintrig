#include "spec_1987.h"

int fintrig_parse_iso_8583_1987(const __u8 *data,
    __u16 len,
    struct fintrig_msg *out) {
    if (len < 2) {
        return -1;
    }
    __u16 mli = data[0] << 8 | data[1];
    if (len < mli + 2) {
        return -1;
    }
    __u16 requiredSize = 0;

    #pragma clang loop unroll(full)
    for (int i = 0; i < FINTRIG_MAX_FIELDS; i++) {
        if (!iso_8583_1987_spec.fields[i].required)
            break;
        requiredSize += iso_8583_1987_spec.fields[i].max_len;
    }

    if (mli < requiredSize) {
        return -1;
    }

    return mli;
}