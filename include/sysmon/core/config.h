#ifndef SYSMON_CORE_CONFIG_H
#define SYSMON_CORE_CONFIG_H

#include <stdint.h>

typedef struct sysmon_core_config {
    const char *mode;
    uint64_t default_interval_ms;
} sysmon_core_config_t;

int sysmon_core_config_init(sysmon_core_config_t *config);

#endif
