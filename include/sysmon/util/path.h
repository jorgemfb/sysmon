#ifndef SYSMON_UTIL_PATH_H
#define SYSMON_UTIL_PATH_H

#include <stddef.h>

int sysmon_path_join(char *dst, size_t dst_size, const char *a, const char *b);

#endif
