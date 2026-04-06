#include "sysmon/report/template.h"

#include <stddef.h>
#include <stdio.h>
#include <time.h>

const char *sysmon_report_default_title(void) {
    return "Sysmon Report";
}

int sysmon_report_format_timestamp(char *buffer, size_t buffer_size) {
    time_t now;
    struct tm tm_now;

    if (buffer == NULL || buffer_size == 0) {
        return -1;
    }

    now = time(NULL);
    if (now == (time_t)-1) {
        (void)snprintf(buffer, buffer_size, "unknown");
        return -1;
    }

    if (localtime_r(&now, &tm_now) == NULL) {
        (void)snprintf(buffer, buffer_size, "unknown");
        return -1;
    }

    if (strftime(buffer, buffer_size, "%Y-%m-%d %H:%M:%S %Z", &tm_now) == 0) {
        (void)snprintf(buffer, buffer_size, "unknown");
        return -1;
    }

    return 0;
}
