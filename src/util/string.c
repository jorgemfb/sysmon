#include "sysmon/util/string.h"

#include <stddef.h>
#include <string.h>

int sysmon_str_copy(char *dst, size_t dst_size, const char *src) {
    size_t len;

    if (dst == NULL || src == NULL || dst_size == 0) {
        return -1;
    }

    len = strlen(src);
    if (len + 1 > dst_size) {
        return -2;
    }

    memcpy(dst, src, len + 1);
    return 0;
}
