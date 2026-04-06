#ifndef SYSMON_CORE_REGISTRY_H
#define SYSMON_CORE_REGISTRY_H

#include <stddef.h>

#include "sysmon/core/module.h"

#define SYSMON_REGISTRY_MAX_MODULES 32

typedef struct sysmon_registry {
    sysmon_module_t modules[SYSMON_REGISTRY_MAX_MODULES];
    size_t count;
} sysmon_registry_t;

int sysmon_registry_init(sysmon_registry_t *registry);
int sysmon_registry_add(sysmon_registry_t *registry, const sysmon_module_t *module);
sysmon_module_t *sysmon_registry_get(sysmon_registry_t *registry, size_t index);

#endif
