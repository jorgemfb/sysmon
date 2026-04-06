#ifndef SYSMON_UTIL_FILE_H
#define SYSMON_UTIL_FILE_H

#include <stddef.h>

int sysmon_file_read_first_line(const char *path, char *buffer, size_t buffer_size);

#endif
