#include "sysmon/modules/identity/identity_module.h"
#include "sysmon/collectors/identity/identity.h"
#include "sysmon/config/defaults.h"

static int sysmon_identity_init(sysmon_module_t *module, struct sysmon_context *context) {
    if (module == NULL || context == NULL) {
        return -1;
    }

    module->interval_ms = context->config.default_interval_ms;
    if (module->interval_ms == 0) {
        module->interval_ms = SYSMON_DEFAULT_INTERVAL_MS;
    }

    return 0;
}

static int sysmon_identity_collect(sysmon_module_t *module, struct sysmon_snapshot *snapshot) {
    (void)module;
    return sysmon_collect_identity(snapshot);
}

static void sysmon_identity_shutdown(sysmon_module_t *module) {
    (void)module;
}

sysmon_module_t sysmon_identity_module_create(void) {
    sysmon_module_t module;

    module.name = "identity";
    module.interval_ms = SYSMON_DEFAULT_INTERVAL_MS;
    module.userdata = NULL;
    module.init = sysmon_identity_init;
    module.collect = sysmon_identity_collect;
    module.shutdown = sysmon_identity_shutdown;

    return module;
}
