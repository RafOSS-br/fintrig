#include "fintrig.h"
#include <types.h>

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

    int byte_index = (field - 1) / 8;
    int bit_index = (field - 1) % 8;

    return (msg->bitmap[byte_index] & (1 << bit_index)) != 0;
}

// static __inline const uint8_t* fintrig_get_field_ptr(const struct fintrig_msg *msg, int field) {

// }

// static __inline int fintrig_get_field_len(const struct fintrig_msg *msg, int field) {

// }
