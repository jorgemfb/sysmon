#include "sysmon/modules/cpu/cpu_module.h"
#include "sysmon/collectors/cpu/cpu.h"
#include "sysmon/config/defaults.h"

static int sysmon_cpu_init(sysmon_module_t *module, struct sysmon_context *context) {
    if (module == NULL || context == NULL) {
        return -1;
    }

    module->interval_ms = context->config.default_interval_ms;
    if (module->interval_ms == 0) {
        module->interval_ms = SYSMON_DEFAULT_INTERVAL_MS;
    }

    return 0;
}

static int sysmon_cpu_collect(sysmon_module_t *module, struct sysmon_snapshot *snapshot) {
    (void)module;
    return sysmon_collect_cpu(snapshot);
}

static void sysmon_cpu_shutdown(sysmon_module_t *module) {
    (void)module;
}

sysmon_module_t sysmon_cpu_module_create(void) {
    sysmon_module_t module;

    module.name = "cpu";
    module.interval_ms = SYSMON_DEFAULT_INTERVAL_MS;
    module.userdata = NULL;
    module.init = sysmon_cpu_init;
    module.collect = sysmon_cpu_collect;
    module.shutdown = sysmon_cpu_shutdown;

    return module;
}
