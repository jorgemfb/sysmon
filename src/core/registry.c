#include "sysmon/core/registry.h"

#include <stddef.h>

int sysmon_registry_init(sysmon_registry_t *registry) {
    if (registry == NULL) {
        return -1;
    }

    registry->count = 0;
    return 0;
}

int sysmon_registry_add(sysmon_registry_t *registry, const sysmon_module_t *module) {
    if (registry == NULL || module == NULL || !sysmon_module_is_valid(module)) {
        return -1;
    }

    if (registry->count >= SYSMON_REGISTRY_MAX_MODULES) {
        return -2;
    }

    registry->modules[registry->count++] = *module;
    return 0;
}

sysmon_module_t *sysmon_registry_get(sysmon_registry_t *registry, size_t index) {
    if (registry == NULL || index >= registry->count) {
        return NULL;
    }

    return &registry->modules[index];
}
