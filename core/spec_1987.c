#include "fintrig.h"

int fintrig_parse_iso_8583(const __u8 *data, __u16 len,
                           const struct fintrig_message_spec *spec,
                           struct fintrig_msg *out) {
    if (!data || !spec || !spec->fields)
        return -1;
    if (len < 2)
        return -1;

    __u16 mli = (data[0] << 8) | data[1];
    if (len < mli + 2)
        return -1;

    __u16 requiredSize = 0;
    for (int i = 0; i < FINTRIG_MAX_FIELDS; i++) {
        if (spec->fields[i].required)
            requiredSize += spec->fields[i].max_len;
    }

    if (mli < requiredSize)
        return -1;

    if (out) {
        out->payload = data + 2;
        out->len = mli;
        out->spec = spec;
    }

    return mli;
}
