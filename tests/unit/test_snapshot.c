#include "sysmon/collectors/identity/identity.h"
#include "sysmon/core/snapshot.h"

#include <stdio.h>
#include <string.h>

static int g_failures = 0;

static void expect_true(int condition, const char *message) {
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", message);
        g_failures++;
    }
}

int main(void) {
    sysmon_snapshot_t snapshot;
    int rc;

    expect_true(sysmon_snapshot_init(NULL) == -1, "snapshot_init(NULL) must fail");

    memset(&snapshot, 0xAB, sizeof(snapshot));
    expect_true(sysmon_snapshot_init(&snapshot) == 0, "snapshot_init must succeed");
    expect_true(snapshot.sample_count == 0, "snapshot sample_count must start at 0");
    expect_true(snapshot.identity_hostname[0] == '\0', "snapshot hostname must start empty");
    expect_true(snapshot.identity_username[0] == '\0', "snapshot username must start empty");
    expect_true(snapshot.identity_kernel[0] == '\0', "snapshot kernel must start empty");
    expect_true(snapshot.identity_uptime_seconds == 0, "snapshot uptime must start at 0");

    rc = sysmon_collect_identity(&snapshot);
    expect_true(rc == 0, "identity collector must succeed");
    expect_true(snapshot.sample_count == 1, "identity collector must increment sample_count");
    expect_true(snapshot.identity_hostname[0] != '\0', "identity collector must store hostname");
    expect_true(snapshot.identity_username[0] != '\0', "identity collector must store username");
    expect_true(snapshot.identity_kernel[0] != '\0', "identity collector must store kernel");

    if (g_failures != 0) {
        fprintf(stderr, "test_snapshot: %d failure(s)\n", g_failures);
        return 1;
    }

    printf("test_snapshot: ok\n");
    return 0;
}
