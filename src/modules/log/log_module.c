#include "sysmon/modules/log/log_module.h"
#include "sysmon/collectors/logs/logs.h"
#include "sysmon/config/defaults.h"

static int sysmon_log_init(sysmon_module_t *module, struct sysmon_context *context) {
    if (module == NULL || context == NULL) {
        return -1;
    }

    module->interval_ms = context->config.default_interval_ms;
    if (module->interval_ms == 0) {
        module->interval_ms = SYSMON_DEFAULT_INTERVAL_MS;
    }

    return 0;
}

static int sysmon_log_collect(sysmon_module_t *module, struct sysmon_snapshot *snapshot) {
    (void)module;
    return sysmon_collect_logs(snapshot);
}

static void sysmon_log_shutdown(sysmon_module_t *module) {
    (void)module;
}

sysmon_module_t sysmon_log_module_create(void) {
    sysmon_module_t module;

    module.name = "log";
    module.interval_ms = SYSMON_DEFAULT_INTERVAL_MS;
    module.userdata = NULL;
    module.init = sysmon_log_init;
    module.collect = sysmon_log_collect;
    module.shutdown = sysmon_log_shutdown;

    return module;
}
