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
src/alert/alert.c \
src/alert/evaluator.c \
src/alert/rule.c \
src/config/defaults.c \
src/config/env.c \
src/config/parser.c \
src/report/markdown.c \
src/report/renderer.c \
src/report/report.c \
src/report/template.c \
src/transport/client.c \
src/transport/message.c \
src/transport/protocol.c \
src/transport/serializer.c \
src/transport/server.c \
src/transport/socket.c \
src/ui/app.c \
src/util/file.c \
src/util/format.c \
src/util/log.c \
src/util/path.c \
src/util/string.c \
src/util/time.c \
src/util/types.c \
src/collectors/cpu/cpu.c \
src/collectors/disk/disk.c \
src/collectors/filesystem/filesystem.c \
src/collectors/identity/identity.c \
src/collectors/logs/logs.c \
src/collectors/memory/memory.c \
src/collectors/network/network.c \
src/collectors/package/package.c \
src/collectors/process/process.c \
src/collectors/service/service.c \
src/collectors/smart/smart.c \
src/collectors/temperature/temperature.c \
src/modules/cpu/cpu_module.c \
src/modules/disk/disk_module.c \
src/modules/filesystem/filesystem_module.c \
src/modules/identity/identity_module.c \
src/modules/log/log_module.c \
src/modules/memory/memory_module.c \
src/modules/network/network_module.c \
src/modules/package/package_module.c \
src/modules/process/process_module.c \
src/modules/report/report_module.c \
src/modules/service/service_module.c \
src/modules/smart/smart_module.c \
src/modules/temperature/temperature_module.c \
src/modules/trend/trend_module.c

OBJ = $(SRC:.c=.o)
TEST_BIN_DIR = build/tests
TEST_NAMES = test_registry test_scheduler test_snapshot test_string test_path test_time test_transport test_socket test_server test_client test_ui
TEST_BINS = $(addprefix $(TEST_BIN_DIR)/,$(TEST_NAMES))
TEST_RUNTIME_SRC = \
src/core/app.c \
src/core/config.c \
src/core/context.c \
src/core/module.c \
src/core/registry.c \
src/core/scheduler.c \
src/core/snapshot.c \
src/alert/alert.c \
src/alert/evaluator.c \
src/alert/rule.c \
src/config/defaults.c \
src/report/markdown.c \
src/report/renderer.c \
src/report/report.c \
src/report/template.c \
src/transport/client.c \
src/transport/message.c \
src/transport/protocol.c \
src/transport/serializer.c \
src/transport/server.c \
src/transport/socket.c \
src/ui/app.c \
src/collectors/cpu/cpu.c \
src/collectors/disk/disk.c \
src/collectors/filesystem/filesystem.c \
src/collectors/identity/identity.c \
src/collectors/logs/logs.c \
src/collectors/memory/memory.c \
src/collectors/network/network.c \
src/collectors/package/package.c \
src/collectors/process/process.c \
src/collectors/service/service.c \
src/collectors/smart/smart.c \
src/collectors/temperature/temperature.c \
src/modules/cpu/cpu_module.c \
src/modules/disk/disk_module.c \
src/modules/filesystem/filesystem_module.c \
src/modules/identity/identity_module.c \
src/modules/log/log_module.c \
src/modules/memory/memory_module.c \
src/modules/network/network_module.c \
src/modules/package/package_module.c \
src/modules/process/process_module.c \
src/modules/report/report_module.c \
src/modules/service/service_module.c \
src/modules/smart/smart_module.c \
src/modules/temperature/temperature_module.c \
src/modules/trend/trend_module.c \
src/util/path.c \
src/util/string.c \
src/util/time.c

INTEGRATION_NAMES = test_local_mode test_server_mode test_client_mode test_markdown_report
INTEGRATION_BINS = $(addprefix $(TEST_BIN_DIR)/integration_,$(INTEGRATION_NAMES))

.PHONY: all clean test test-integration test-all run

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

$(TEST_BIN_DIR)/integration_%: tests/integration/%.c $(TEST_RUNTIME_SRC) | $(TEST_BIN_DIR)
	$(CC) $(CFLAGS) $< $(TEST_RUNTIME_SRC) -o $@

test: $(TEST_BINS)
	@set -e; \
	for test_bin in $(TEST_BINS); do \
		echo "Running $$test_bin"; \
		$$test_bin; \
	done

test-integration: $(INTEGRATION_BINS)
	@set -e; \
	for test_bin in $(INTEGRATION_BINS); do \
		echo "Running $$test_bin"; \
		$$test_bin; \
	done

test-all: test test-integration

clean:
	rm -f $(OBJ) $(TARGET)
	rm -rf $(TEST_BIN_DIR)
