#include "sysmon/util/file.h"

#include <stddef.h>
#include <stdio.h>

int sysmon_file_read_first_line(const char *path, char *buffer, size_t buffer_size) {
    FILE *fp;

    if (path == NULL || buffer == NULL || buffer_size == 0) {
        return -1;
    }

    fp = fopen(path, "r");
    if (fp == NULL) {
        return -2;
    }

    if (fgets(buffer, (int)buffer_size, fp) == NULL) {
        fclose(fp);
        return -3;
    }

    fclose(fp);
    return 0;
}
