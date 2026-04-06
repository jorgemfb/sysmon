#include "sysmon/util/string.h"

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
    char buffer[16];
    int rc;

    rc = sysmon_str_copy(buffer, sizeof(buffer), "sysmon");
    expect_true(rc == 0, "str_copy valid input must succeed");
    expect_true(strcmp(buffer, "sysmon") == 0, "str_copy must copy full string");

    expect_true(sysmon_str_copy(NULL, sizeof(buffer), "x") == -1, "str_copy NULL dst must fail");
    expect_true(sysmon_str_copy(buffer, 0, "x") == -1, "str_copy zero size must fail");
    expect_true(sysmon_str_copy(buffer, sizeof(buffer), NULL) == -1, "str_copy NULL src must fail");
    expect_true(sysmon_str_copy(buffer, 4, "toolong") == -2, "str_copy overflow must fail");

    if (g_failures != 0) {
        fprintf(stderr, "test_string: %d failure(s)\n", g_failures);
        return 1;
    }

    printf("test_string: ok\n");
    return 0;
}
