#include "sysmon/collectors/network/network.h"

#include <stddef.h>
#include <stdio.h>
#include <string.h>

static void sysmon_network_copy_text(char *dst, size_t dst_size, const char *src) {
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

static void sysmon_network_trim(char *text) {
    size_t len;

    while (*text == ' ' || *text == '\t') {
        memmove(text, text + 1, strlen(text));
    }

    len = strlen(text);
    while (len > 0 && (text[len - 1] == ' ' || text[len - 1] == '\t' || text[len - 1] == '\n' || text[len - 1] == '\r')) {
        text[len - 1] = '\0';
        len--;
    }
}

static void sysmon_network_set_oper_state(sysmon_network_entry_t *entry, const char *ifname) {
    char path[256];
    FILE *fp;
    char line[64];

    (void)snprintf(path, sizeof(path), "/sys/class/net/%s/operstate", ifname);
    fp = fopen(path, "r");
    if (fp == NULL) {
        sysmon_network_copy_text(entry->oper_state, sizeof(entry->oper_state), "unknown");
        return;
    }

    if (fgets(line, sizeof(line), fp) == NULL) {
        fclose(fp);
        sysmon_network_copy_text(entry->oper_state, sizeof(entry->oper_state), "unknown");
        return;
    }

    fclose(fp);
    sysmon_network_trim(line);
    sysmon_network_copy_text(entry->oper_state, sizeof(entry->oper_state), line[0] != '\0' ? line : "unknown");
}

int sysmon_collect_network(sysmon_snapshot_t *snapshot) {
    FILE *fp;
    char line[512];
    size_t count = 0;

    if (snapshot == NULL) {
        return -1;
    }

    snapshot->network_count = 0;

    fp = fopen("/proc/net/dev", "r");
    if (fp == NULL) {
        return -2;
    }

    while (fgets(line, sizeof(line), fp) != NULL) {
        char *colon;
        char ifname[64];
        unsigned long long values[16] = {0ULL};
        int scanned;

        if (strncmp(line, "Inter-|", 7) == 0 || strncmp(line, " face |", 7) == 0) {
            continue;
        }

        colon = strchr(line, ':');
        if (colon == NULL) {
            continue;
        }

        *colon = '\0';
        sysmon_network_copy_text(ifname, sizeof(ifname), line);
        sysmon_network_trim(ifname);
        if (ifname[0] == '\0') {
            continue;
        }

        scanned = sscanf(colon + 1,
                         "%llu %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu",
                         &values[0], &values[1], &values[2], &values[3], &values[4], &values[5], &values[6],
                         &values[7], &values[8], &values[9], &values[10], &values[11], &values[12], &values[13],
                         &values[14], &values[15]);
        if (scanned < 9) {
            continue;
        }

        sysmon_network_copy_text(snapshot->network_entries[count].interface_name,
                                 sizeof(snapshot->network_entries[count].interface_name),
                                 ifname);
        snapshot->network_entries[count].rx_bytes = (uint64_t)values[0];
        snapshot->network_entries[count].tx_bytes = (uint64_t)values[8];
        sysmon_network_set_oper_state(&snapshot->network_entries[count], ifname);

        count++;
        if (count >= SYSMON_NETWORK_MAX_ENTRIES) {
            break;
        }
    }

    fclose(fp);

    snapshot->network_count = count;
    snapshot->sample_count++;

    return 0;
}
