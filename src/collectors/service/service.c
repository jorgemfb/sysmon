#include "sysmon/collectors/service/service.h"

#include <stddef.h>
#include <stdio.h>
#include <string.h>

int sysmon_collect_service(sysmon_snapshot_t *snapshot) {
    FILE *fp;
    char line[512];
    uint64_t total = 0;
    uint64_t active = 0;
    uint64_t failed = 0;
    const char *cmd =
        "systemctl list-units --type=service --all --no-legend --no-pager --plain 2>/dev/null";

    if (snapshot == NULL) {
        return -1;
    }

    fp = popen(cmd, "r");
    if (fp != NULL) {
        while (fgets(line, sizeof(line), fp) != NULL) {
            char unit[256];
            char load[64];
            char active_state[64];
            char sub_state[64];
            int scanned;

            scanned = sscanf(line, "%255s %63s %63s %63s", unit, load, active_state, sub_state);
            if (scanned < 3) {
                continue;
            }

            if (strstr(unit, ".service") == NULL) {
                continue;
            }

            total++;
            if (strcmp(active_state, "active") == 0) {
                active++;
            } else if (strcmp(active_state, "failed") == 0) {
                failed++;
            }
        }

        (void)pclose(fp);
    }

    snapshot->service_total_count = total;
    snapshot->service_active_count = active;
    snapshot->service_failed_count = failed;
    snapshot->sample_count++;

    return 0;
}
