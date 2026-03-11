CC ?= gcc
CFLAGS ?= -std=c11 -Wall -Wextra -Werror -pedantic -Isrc
LDFLAGS ?= -lm
SRC := $(wildcard src/*.c)
TEST_BIN := tests/test_wcx
TEST_SRC := tests/test_wcx.c

.PHONY: test clean

test: $(TEST_BIN)
	./$(TEST_BIN)

$(TEST_BIN): $(TEST_SRC) $(SRC)
	$(CC) $(CFLAGS) $(TEST_SRC) $(SRC) -o $(TEST_BIN) $(LDFLAGS)

clean:
	$(RM) $(TEST_BIN)
