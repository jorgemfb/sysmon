#include "sysmon/core/scheduler.h"
#include "sysmon/util/time.h"

#include <stddef.h>
#include <string.h>

int sysmon_scheduler_init(sysmon_scheduler_t *scheduler) {
    if (scheduler == NULL) {
        return -1;
    }

    scheduler->running = true;
    memset(scheduler->last_run_ms, 0, sizeof(scheduler->last_run_ms));

    return 0;
}

int sysmon_scheduler_run_once(sysmon_scheduler_t *scheduler, sysmon_registry_t *registry, sysmon_snapshot_t *snapshot) {
    size_t i;
    uint64_t now_ms = sysmon_time_now_ms();

    if (scheduler == NULL || registry == NULL || snapshot == NULL) {
        return -1;
    }

    if (!scheduler->running) {
        return 0;
    }

    for (i = 0; i < registry->count; ++i) {
        sysmon_module_t *module = sysmon_registry_get(registry, i);
        uint64_t interval_ms;
        int collect_rc;

        if (module != NULL && module->collect != NULL) {
            interval_ms = module->interval_ms != 0 ? module->interval_ms : 1000ULL;

            if (scheduler->last_run_ms[i] != 0 && (now_ms - scheduler->last_run_ms[i]) < interval_ms) {
                continue;
            }

            collect_rc = module->collect(module, snapshot);
            if (collect_rc != 0) {
                return -2;
            }

            scheduler->last_run_ms[i] = now_ms;
        }
    }

    return 0;
}
