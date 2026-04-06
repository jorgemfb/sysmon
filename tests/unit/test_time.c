#include "sysmon/util/time.h"

#include <stdio.h>

static int g_failures = 0;

static void expect_true(int condition, const char *message) {
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", message);
        g_failures++;
    }
}

int main(void) {
    uint64_t before;
    uint64_t after;

    before = sysmon_time_now_ms();
    expect_true(before > 0, "time_now_ms must return a positive value");
    expect_true(sysmon_time_sleep_ms(5) == 0, "time_sleep_ms must succeed");
    after = sysmon_time_now_ms();
    expect_true(after >= before, "time_now_ms must be monotonic");
    expect_true((after - before) >= 1, "time difference after sleep must be at least 1 ms");
    expect_true(sysmon_time_sleep_ms(0) == 0, "time_sleep_ms(0) must succeed");

    if (g_failures != 0) {
        fprintf(stderr, "test_time: %d failure(s)\n", g_failures);
        return 1;
    }

    printf("test_time: ok\n");
    return 0;
}
