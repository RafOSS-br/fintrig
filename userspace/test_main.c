#include <stdio.h>

extern int test_parser();
extern int test_fields();
extern int test_bitmap();
extern int test_validate();

int main() {
    int fail = 0;
    fail |= test_parser();
    fail |= test_validate();
    fail |= test_fields();
    fail |= test_bitmap();

    if (fail) {
        printf("❌ Some tests failed\n");
        return 1;
    }

    printf("✅ All tests passed\n");
    return 0;
}
