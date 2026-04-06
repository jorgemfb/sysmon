#include "sysmon/modules/smart/smart_module.h"
#include "sysmon/collectors/smart/smart.h"
#include "sysmon/config/defaults.h"

static int sysmon_smart_init(sysmon_module_t *module, struct sysmon_context *context) {
    if (module == NULL || context == NULL) {
        return -1;
    }

    module->interval_ms = context->config.default_interval_ms;
    if (module->interval_ms == 0) {
        module->interval_ms = SYSMON_DEFAULT_INTERVAL_MS;
    }

    return 0;
}

static int sysmon_smart_collect(sysmon_module_t *module, struct sysmon_snapshot *snapshot) {
    (void)module;
    return sysmon_collect_smart(snapshot);
}

static void sysmon_smart_shutdown(sysmon_module_t *module) {
    (void)module;
}

sysmon_module_t sysmon_smart_module_create(void) {
    sysmon_module_t module;

    module.name = "smart";
    module.interval_ms = SYSMON_DEFAULT_INTERVAL_MS;
    module.userdata = NULL;
    module.init = sysmon_smart_init;
    module.collect = sysmon_smart_collect;
    module.shutdown = sysmon_smart_shutdown;

    return module;
}
