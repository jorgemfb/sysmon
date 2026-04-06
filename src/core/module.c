#include "sysmon/core/module.h"

#include <stddef.h>

int sysmon_module_is_valid(const sysmon_module_t *module) {
    if (module == NULL || module->name == NULL || module->collect == NULL) {
        return 0;
    }

    return 1;
}
