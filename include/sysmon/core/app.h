#ifndef SYSMON_CORE_APP_H
#define SYSMON_CORE_APP_H

#include "sysmon/core/context.h"
#include "sysmon/core/registry.h"
#include "sysmon/core/scheduler.h"
#include "sysmon/core/snapshot.h"

typedef struct sysmon_app {
    sysmon_context_t context;
    sysmon_registry_t registry;
    sysmon_scheduler_t scheduler;
    sysmon_snapshot_t snapshot;
} sysmon_app_t;

int sysmon_app_init(sysmon_app_t *app);
int sysmon_app_register_defaults(sysmon_app_t *app);
int sysmon_app_run(sysmon_app_t *app);
void sysmon_app_shutdown(sysmon_app_t *app);

#endif
