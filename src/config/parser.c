#include "sysmon/config/parser.h"

#include <stddef.h>

int sysmon_config_parse_file(const char *path, sysmon_config_file_t *out) {
    (void)path;

    if (out == NULL) {
        return -1;
    }

    out->mode = "local";
    return 0;
}
