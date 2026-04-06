#include "sysmon/util/path.h"

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
    char path[64];
    int rc;

    rc = sysmon_path_join(path, sizeof(path), "/tmp", "file.txt");
    expect_true(rc == 0, "path_join valid input must succeed");
    expect_true(strcmp(path, "/tmp/file.txt") == 0, "path_join must create expected path");

    expect_true(sysmon_path_join(NULL, sizeof(path), "/a", "b") == -1, "path_join NULL dst must fail");
    expect_true(sysmon_path_join(path, 0, "/a", "b") == -1, "path_join zero size must fail");
    expect_true(sysmon_path_join(path, sizeof(path), NULL, "b") == -1, "path_join NULL a must fail");
    expect_true(sysmon_path_join(path, sizeof(path), "/a", NULL) == -1, "path_join NULL b must fail");
    expect_true(sysmon_path_join(path, 8, "/abcdef", "ghij") == -2, "path_join overflow must fail");

    if (g_failures != 0) {
        fprintf(stderr, "test_path: %d failure(s)\n", g_failures);
        return 1;
    }

    printf("test_path: ok\n");
    return 0;
}
