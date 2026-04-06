#include "sysmon/modules/report/report_module.h"

#include "sysmon/alert/evaluator.h"
#include "sysmon/config/defaults.h"
#include "sysmon/report/report.h"

static int sysmon_report_init(sysmon_module_t *module, struct sysmon_context *context) {
    if (module == NULL || context == NULL) {
        return -1;
    }

    module->interval_ms = context->config.default_interval_ms;
    if (module->interval_ms == 0) {
        module->interval_ms = SYSMON_DEFAULT_INTERVAL_MS;
    }

    return 0;
}

static int sysmon_report_collect(sysmon_module_t *module, struct sysmon_snapshot *snapshot) {
    int rc;

    (void)module;
    rc = sysmon_alert_evaluate(snapshot);
    if (rc != 0) {
        return rc;
    }

    rc = sysmon_report_generate(snapshot);
    if (rc != 0) {
        return rc;
    }

    snapshot->sample_count++;
    return 0;
}

static void sysmon_report_shutdown(sysmon_module_t *module) {
    (void)module;
}

sysmon_module_t sysmon_report_module_create(void) {
    sysmon_module_t module;

    module.name = "report";
    module.interval_ms = SYSMON_DEFAULT_INTERVAL_MS;
    module.userdata = NULL;
    module.init = sysmon_report_init;
    module.collect = sysmon_report_collect;
    module.shutdown = sysmon_report_shutdown;

    return module;
}
