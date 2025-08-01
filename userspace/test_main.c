#include <stdio.h>

extern int test_parser();
extern int test_has_field();
extern int test_bitmap();
extern int test_validate();
extern int test_has_secondary_bitmap();
extern int test_has_field_with_secondary();
extern int test_bitmap_parsing();
extern int test_secondary_bitmap_presence();
extern int test_field_detection_primary();
extern int test_field_detection_secondary();
extern int test_field_pointer_and_buckets();
extern int test_bucket_initial_build();
extern int test_bucket_incremental_build();
extern int test_bucket_reuse();
extern int test_field_object_basic();
extern int test_field_object_tlv_iteration();

int main() {
    int fail = 0;
    fail |= test_parser();
    fail |= test_validate();
    fail |= test_has_field();
    fail |= test_bitmap();
    fail |= test_has_secondary_bitmap();
    fail |= test_has_field_with_secondary();
    fail |= test_bitmap_parsing();
    fail |= test_secondary_bitmap_presence();
    fail |= test_field_detection_primary();
    fail |= test_field_detection_secondary();
    fail |= test_field_pointer_and_buckets();
    fail |= test_bucket_initial_build();
    fail |= test_bucket_incremental_build();
    fail |= test_bucket_reuse();
    fail |= test_field_object_basic();
    fail |= test_field_object_tlv_iteration();
    if (fail) {
        printf("❌ Some tests failed\n");
        return 1;
    }

    printf("✅ All tests passed\n");
    return 0;
}
