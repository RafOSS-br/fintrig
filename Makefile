# For release:
# CFLAGS = -Wall -Wextra -O3 -flto -g -std=c11 -Rpass=loop-unroll -I./core -I./userspace
# LDFLAGS = -Wl,-plugin-opt=save-temps

# For debug:
CFLAGS = -Wall -Wextra -O0 -g -std=c11 -I./core -I./userspace
LDFLAGS =


TEST_SRC = $(wildcard userspace/test_*.c)
TEST_OBJ = $(TEST_SRC:.c=.o)
TEST_BIN = fintrig-tests

.PHONY: all clean test

all: $(TEST_BIN)

$(TEST_BIN): $(TEST_OBJ) core/fintrig.o core/spec_1987.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

core/fintrig.o: core/fintrig.c
	$(CC) $(CFLAGS) -c -o $@ $<

core/spec_1987.o: core/spec_1987.c
	$(CC) $(CFLAGS) -c -o $@ $<

test: $(TEST_BIN)
	./$(TEST_BIN)

clean:
	rm -f $(TEST_OBJ) core/*.o core/*.ll $(TEST_BIN)
	rm -f $(TEST_BIN)*.bc $(TEST_BIN)*.txt $(TEST_BIN)*.lto.o $(TEST_BIN)*.ll
