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

    if (spec->fields[0].max_len + spec->fields[1].max_len > mli)
        return -1;

    data += 2; // Skip MLI
    data += spec->fields[0].max_len; // Skip MTI

    __u8 bitmap[8];
    __builtin_memcpy(bitmap, data, 8);

    if (out) {
        for (int i = 0; i < 8; i++)
            out->bitmap[i] = bitmap[i];

        out->payload = data + 8;
        out->len = mli;
        out->spec = spec;
    }

    return mli;
}
