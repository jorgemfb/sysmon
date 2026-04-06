#include "sysmon/collectors/process/process.h"

#include <ctype.h>
#include <dirent.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

static int sysmon_process_is_numeric_name(const char *name) {
    size_t i;

    if (name == NULL || name[0] == '\0') {
        return 0;
    }

    for (i = 0; name[i] != '\0'; ++i) {
        if (!isdigit((unsigned char)name[i])) {
            return 0;
        }
    }

    return 1;
}

static int sysmon_process_read_state(const char *pid_name, char *out_state) {
    char path[256];
    FILE *fp;
    char line[1024];
    char *close_paren;
    char state = '\0';

    (void)snprintf(path, sizeof(path), "/proc/%s/stat", pid_name);
    fp = fopen(path, "r");
    if (fp == NULL) {
        return -1;
    }

    if (fgets(line, sizeof(line), fp) == NULL) {
        fclose(fp);
        return -2;
    }

    fclose(fp);

    close_paren = strrchr(line, ')');
    if (close_paren == NULL) {
        return -3;
    }

    if (sscanf(close_paren + 1, " %c", &state) != 1) {
        return -4;
    }

    *out_state = state;
    return 0;
}

int sysmon_collect_process(sysmon_snapshot_t *snapshot) {
    DIR *proc_dir;
    struct dirent *entry;
    uint64_t total = 0;
    uint64_t running = 0;
    uint64_t sleeping = 0;

    if (snapshot == NULL) {
        return -1;
    }

    proc_dir = opendir("/proc");
    if (proc_dir == NULL) {
        return -2;
    }

    while ((entry = readdir(proc_dir)) != NULL) {
        char state = '\0';

        if (!sysmon_process_is_numeric_name(entry->d_name)) {
            continue;
        }

        total++;

        if (sysmon_process_read_state(entry->d_name, &state) != 0) {
            continue;
        }

        if (state == 'R') {
            running++;
        } else if (state == 'S' || state == 'D' || state == 'I') {
            sleeping++;
        }
    }

    closedir(proc_dir);

    snapshot->process_total_count = total;
    snapshot->process_running_count = running;
    snapshot->process_sleeping_count = sleeping;
    snapshot->sample_count++;

    return 0;
}
