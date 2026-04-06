#include "sysmon/util/format.h"

#include <stddef.h>
#include <stdio.h>

int sysmon_format_bool(char *dst, size_t dst_size, int value) {
    if (dst == NULL || dst_size == 0) {
        return -1;
    }

    return (snprintf(dst, dst_size, "%s", value ? "true" : "false") >= (int)dst_size) ? -2 : 0;
}
