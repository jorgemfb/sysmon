#include "sysmon/modules/disk/disk_module.h"
#include "sysmon/collectors/disk/disk.h"
#include "sysmon/config/defaults.h"

static int sysmon_disk_init(sysmon_module_t *module, struct sysmon_context *context) {
    if (module == NULL || context == NULL) {
        return -1;
    }

    module->interval_ms = context->config.default_interval_ms;
    if (module->interval_ms == 0) {
        module->interval_ms = SYSMON_DEFAULT_INTERVAL_MS;
    }

    return 0;
}

static int sysmon_disk_collect(sysmon_module_t *module, struct sysmon_snapshot *snapshot) {
    (void)module;
    return sysmon_collect_disk(snapshot);
}

static void sysmon_disk_shutdown(sysmon_module_t *module) {
    (void)module;
}

sysmon_module_t sysmon_disk_module_create(void) {
    sysmon_module_t module;

    module.name = "disk";
    module.interval_ms = SYSMON_DEFAULT_INTERVAL_MS;
    module.userdata = NULL;
    module.init = sysmon_disk_init;
    module.collect = sysmon_disk_collect;
    module.shutdown = sysmon_disk_shutdown;

    return module;
}
