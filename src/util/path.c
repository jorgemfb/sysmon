#include "sysmon/util/path.h"

#include <stddef.h>
#include <stdio.h>

int sysmon_path_join(char *dst, size_t dst_size, const char *a, const char *b) {
    if (dst == NULL || a == NULL || b == NULL || dst_size == 0) {
        return -1;
    }

    return (snprintf(dst, dst_size, "%s/%s", a, b) >= (int)dst_size) ? -2 : 0;
}
