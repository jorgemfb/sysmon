#include "sysmon/collectors/logs/logs.h"

#include <ctype.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SYSMON_LOG_SCAN_LIMIT 200

static void sysmon_logs_copy_text(char *dst, size_t dst_size, const char *src) {
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

static void sysmon_logs_trim(char *text) {
    size_t len;

    while (*text == ' ' || *text == '\t') {
        memmove(text, text + 1, strlen(text));
    }

    len = strlen(text);
    while (len > 0 && (text[len - 1] == '\n' || text[len - 1] == '\r' || text[len - 1] == ' ' || text[len - 1] == '\t')) {
        text[len - 1] = '\0';
        len--;
    }
}

static void sysmon_logs_lowercase_copy(char *dst, size_t dst_size, const char *src) {
    size_t i = 0;

    if (dst_size == 0) {
        return;
    }

    if (src == NULL) {
        dst[0] = '\0';
        return;
    }

    while (src[i] != '\0' && i + 1 < dst_size) {
        dst[i] = (char)tolower((unsigned char)src[i]);
        i++;
    }

    dst[i] = '\0';
}

static int sysmon_logs_has_journalctl(void) {
    return system("journalctl --version >/dev/null 2>&1") == 0;
}

static int sysmon_logs_classify_level(const char *lower_line) {
    if (strstr(lower_line, "error") != NULL || strstr(lower_line, "failed") != NULL) {
        return 2;
    }

    if (strstr(lower_line, "warning") != NULL || strstr(lower_line, "warn") != NULL) {
        return 1;
    }

    return 0;
}

static void sysmon_logs_store_message(sysmon_snapshot_t *snapshot, const char *level, const char *message) {
    size_t i;
    size_t idx;

    if (snapshot->log_count >= SYSMON_LOG_MAX_MESSAGES) {
        for (i = 1; i < snapshot->log_count; ++i) {
            snapshot->log_entries[i - 1] = snapshot->log_entries[i];
        }
        idx = snapshot->log_count - 1;
    } else {
        idx = snapshot->log_count;
        snapshot->log_count++;
    }

    sysmon_logs_copy_text(snapshot->log_entries[idx].level, sizeof(snapshot->log_entries[idx].level), level);
    sysmon_logs_copy_text(snapshot->log_entries[idx].message, sizeof(snapshot->log_entries[idx].message), message);
}

int sysmon_collect_logs(sysmon_snapshot_t *snapshot) {
    FILE *fp;
    char line[1024];
    char lower_line[1024];
    const char *cmd = "journalctl -n 200 --no-pager --output=short 2>/dev/null";

    if (snapshot == NULL) {
        return -1;
    }

    snapshot->log_total_entries_considered = 0;
    snapshot->log_warning_count = 0;
    snapshot->log_error_count = 0;
    snapshot->log_count = 0;
    snapshot->log_source_available = 0;

    if (!sysmon_logs_has_journalctl()) {
        snapshot->sample_count++;
        return 0;
    }

    snapshot->log_source_available = 1;

    fp = popen(cmd, "r");
    if (fp == NULL) {
        snapshot->sample_count++;
        return 0;
    }

    while (fgets(line, sizeof(line), fp) != NULL) {
        int level;

        sysmon_logs_trim(line);
        if (line[0] == '\0') {
            continue;
        }

        snapshot->log_total_entries_considered++;

        sysmon_logs_lowercase_copy(lower_line, sizeof(lower_line), line);
        level = sysmon_logs_classify_level(lower_line);
        if (level == 2) {
            snapshot->log_error_count++;
            sysmon_logs_store_message(snapshot, "error", line);
        } else if (level == 1) {
            snapshot->log_warning_count++;
            sysmon_logs_store_message(snapshot, "warning", line);
        }

        if (snapshot->log_total_entries_considered >= SYSMON_LOG_SCAN_LIMIT) {
            break;
        }
    }

    (void)pclose(fp);
    snapshot->sample_count++;

    return 0;
}
