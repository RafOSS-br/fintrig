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

// static __inline const uint8_t* fintrig_get_field_ptr(const struct fintrig_msg *msg, int field) {

// }

// static __inline int fintrig_get_field_len(const struct fintrig_msg *msg, int field) {

// }
