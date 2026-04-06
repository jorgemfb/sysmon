#include "sysmon/collectors/temperature/temperature.h"

#include <dirent.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void sysmon_temperature_copy_text(char *dst, size_t dst_size, const char *src) {
    size_t n;

    if (dst_size == 0) {
        return;
    }

    if (src == NULL) {
        dst[0] = '\0';
        return;
    }

    n = strnlen(src, dst_size - 1);
    memcpy(dst, src, n);
    dst[n] = '\0';
}

static void sysmon_temperature_trim(char *text) {
    size_t len = strlen(text);

    while (len > 0 && (text[len - 1] == '\n' || text[len - 1] == '\r' || text[len - 1] == ' ' || text[len - 1] == '\t')) {
        text[len - 1] = '\0';
        len--;
    }
}

static int sysmon_temperature_read_text(const char *path, char *buffer, size_t buffer_size) {
    FILE *fp;

    fp = fopen(path, "r");
    if (fp == NULL) {
        return -1;
    }

    if (fgets(buffer, (int)buffer_size, fp) == NULL) {
        fclose(fp);
        return -2;
    }

    fclose(fp);
    sysmon_temperature_trim(buffer);
    return 0;
}

static int sysmon_temperature_read_double(const char *path, double *value_out) {
    FILE *fp;
    long raw;

    fp = fopen(path, "r");
    if (fp == NULL) {
        return -1;
    }

    if (fscanf(fp, "%ld", &raw) != 1) {
        fclose(fp);
        return -2;
    }

    fclose(fp);

    if (raw > 1000L || raw < -1000L) {
        *value_out = (double)raw / 1000.0;
    } else {
        *value_out = (double)raw;
    }

    return 0;
}

int sysmon_collect_temperature(sysmon_snapshot_t *snapshot) {
    DIR *thermal_dir;
    struct dirent *entry;
    size_t count = 0;

    if (snapshot == NULL) {
        return -1;
    }

    snapshot->temperature_count = 0;

    thermal_dir = opendir("/sys/class/thermal");
    if (thermal_dir == NULL) {
        snapshot->sample_count++;
        return 0;
    }

    while ((entry = readdir(thermal_dir)) != NULL) {
        char type_path[512];
        char temp_path[512];
        char sensor_name[64];
        double temp_c = 0.0;

        if (strncmp(entry->d_name, "thermal_zone", 12) != 0) {
            continue;
        }

        (void)snprintf(type_path, sizeof(type_path), "/sys/class/thermal/%s/type", entry->d_name);
        (void)snprintf(temp_path, sizeof(temp_path), "/sys/class/thermal/%s/temp", entry->d_name);

        if (sysmon_temperature_read_double(temp_path, &temp_c) != 0) {
            continue;
        }

        if (sysmon_temperature_read_text(type_path, sensor_name, sizeof(sensor_name)) != 0 || sensor_name[0] == '\0') {
            sysmon_temperature_copy_text(sensor_name, sizeof(sensor_name), entry->d_name);
        }

        sysmon_temperature_copy_text(snapshot->temperature_entries[count].sensor_name,
                                     sizeof(snapshot->temperature_entries[count].sensor_name),
                                     sensor_name);
        snapshot->temperature_entries[count].temperature_c = temp_c;

        count++;
        if (count >= SYSMON_TEMPERATURE_MAX_ENTRIES) {
            break;
        }
    }

    closedir(thermal_dir);

    snapshot->temperature_count = count;
    snapshot->sample_count++;
    return 0;
}
