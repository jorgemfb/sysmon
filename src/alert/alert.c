#include "sysmon/alert/alert.h"

#include <stddef.h>
#include <string.h>

static void sysmon_alert_copy_text(char *dst, size_t dst_size, const char *src) {
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

int sysmon_alert_reset(sysmon_snapshot_t *snapshot) {
    if (snapshot == NULL) {
        return -1;
    }

    snapshot->alert_count = 0;
    snapshot->alerts_evaluated = 0;
    return 0;
}

int sysmon_alert_add(sysmon_snapshot_t *snapshot, const char *severity, const char *title, const char *message) {
    size_t idx;

    if (snapshot == NULL || severity == NULL || title == NULL || message == NULL) {
        return -1;
    }

    if (snapshot->alert_count >= SYSMON_ALERT_MAX_ENTRIES) {
        return -2;
    }

    idx = snapshot->alert_count;
    sysmon_alert_copy_text(snapshot->alert_entries[idx].severity, sizeof(snapshot->alert_entries[idx].severity), severity);
    sysmon_alert_copy_text(snapshot->alert_entries[idx].title, sizeof(snapshot->alert_entries[idx].title), title);
    sysmon_alert_copy_text(snapshot->alert_entries[idx].message, sizeof(snapshot->alert_entries[idx].message), message);
    snapshot->alert_count++;

    return 0;
}
