#include "sysmon/core/config.h"
#include "sysmon/config/defaults.h"

#include <stddef.h>

int sysmon_core_config_init(sysmon_core_config_t *config) {
    if (config == NULL) {
        return -1;
    }

    config->mode = SYSMON_DEFAULT_MODE;
    config->default_interval_ms = SYSMON_DEFAULT_INTERVAL_MS;
    return 0;
}
