#include "sysmon/util/log.h"

#include <stddef.h>
#include <stdio.h>

void sysmon_log_info(const char *message) {
    if (message != NULL) {
        fprintf(stdout, "[INFO] %s\n", message);
    }
}

void sysmon_log_error(const char *message) {
    if (message != NULL) {
        fprintf(stderr, "[ERROR] %s\n", message);
    }
}
