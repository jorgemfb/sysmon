CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -Wpedantic -D_POSIX_C_SOURCE=200809L -Iinclude
LDFLAGS =
TARGET = bin/sysmon

SRC = \
src/main.c \
src/core/app.c \
src/core/config.c \
src/core/context.c \
src/core/error.c \
src/core/event.c \
src/core/module.c \
src/core/registry.c \
src/core/scheduler.c \
src/core/snapshot.c \
src/config/defaults.c \
src/config/env.c \
src/config/parser.c \
src/util/file.c \
src/util/format.c \
src/util/log.c \
src/util/path.c \
src/util/string.c \
src/util/time.c \
src/util/types.c \
src/collectors/identity/identity.c \
src/modules/identity/identity_module.c

OBJ = $(SRC:.c=.o)
TEST_BIN_DIR = build/tests
TEST_NAMES = test_registry test_scheduler test_snapshot test_string test_path test_time
TEST_BINS = $(addprefix $(TEST_BIN_DIR)/,$(TEST_NAMES))
TEST_RUNTIME_SRC = \
src/core/config.c \
src/core/context.c \
src/core/module.c \
src/core/registry.c \
src/core/scheduler.c \
src/core/snapshot.c \
src/config/defaults.c \
src/collectors/identity/identity.c \
src/modules/identity/identity_module.c \
src/util/path.c \
src/util/string.c \
src/util/time.c

.PHONY: all clean test run

all: $(TARGET)

$(TARGET): $(OBJ)
	@mkdir -p bin
	$(CC) $(OBJ) -o $(TARGET) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

run: $(TARGET)
	./$(TARGET)

$(TEST_BIN_DIR):
	@mkdir -p $(TEST_BIN_DIR)

$(TEST_BIN_DIR)/%: tests/unit/%.c $(TEST_RUNTIME_SRC) | $(TEST_BIN_DIR)
	$(CC) $(CFLAGS) $< $(TEST_RUNTIME_SRC) -o $@

test: $(TEST_BINS)
	@set -e; \
	for test_bin in $(TEST_BINS); do \
		echo "Running $$test_bin"; \
		$$test_bin; \
	done

clean:
	rm -f $(OBJ) $(TARGET)
	rm -rf $(TEST_BIN_DIR)
