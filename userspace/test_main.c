#include <stdio.h>

extern int test_parser();
extern int test_has_field();
extern int test_bitmap();
extern int test_validate();
extern int test_has_secondary_bitmap();
extern int test_has_field_with_secondary();

int main() {
    int fail = 0;
    fail |= test_parser();
    fail |= test_validate();
    fail |= test_has_field();
    fail |= test_bitmap();
    fail |= test_has_secondary_bitmap();
    fail |= test_has_field_with_secondary();

    if (fail) {
        printf("❌ Some tests failed\n");
        return 1;
    }

    printf("✅ All tests passed\n");
    return 0;
}
