#include "sysmon/core/app.h"
#include "sysmon/modules/identity/identity_module.h"
#include "sysmon/util/time.h"

#include <stddef.h>

int sysmon_app_init(sysmon_app_t *app) {
    if (app == NULL) {
        return -1;
    }

    if (sysmon_context_init(&app->context) != 0) {
        return -1;
    }

    if (sysmon_registry_init(&app->registry) != 0) {
        return -1;
    }

    if (sysmon_scheduler_init(&app->scheduler) != 0) {
        return -1;
    }

    if (sysmon_snapshot_init(&app->snapshot) != 0) {
        return -1;
    }

    return 0;
}

int sysmon_app_register_defaults(sysmon_app_t *app) {
    sysmon_module_t module;
    int init_rc = 0;

    if (app == NULL) {
        return -1;
    }

    module = sysmon_identity_module_create();
    if (module.init != NULL) {
        init_rc = module.init(&module, &app->context);
        if (init_rc != 0) {
            return -2;
        }
    }

    return sysmon_registry_add(&app->registry, &module);
}

int sysmon_app_run(sysmon_app_t *app) {
    int rc;

    if (app == NULL) {
        return -1;
    }

    rc = sysmon_scheduler_run_once(&app->scheduler, &app->registry, &app->snapshot);
    if (rc != 0) {
        return rc;
    }

    if (sysmon_time_sleep_ms(app->context.config.default_interval_ms) != 0) {
        return -2;
    }

    return sysmon_scheduler_run_once(&app->scheduler, &app->registry, &app->snapshot);
}

void sysmon_app_shutdown(sysmon_app_t *app) {
    size_t i;

    if (app == NULL) {
        return;
    }

    for (i = 0; i < app->registry.count; ++i) {
        sysmon_module_t *module = sysmon_registry_get(&app->registry, i);
        if (module != NULL && module->shutdown != NULL) {
            module->shutdown(module);
        }
    }
}
