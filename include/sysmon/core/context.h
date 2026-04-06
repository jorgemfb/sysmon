#ifndef SYSMON_CORE_CONTEXT_H
#define SYSMON_CORE_CONTEXT_H

#include "sysmon/core/config.h"

typedef struct sysmon_context {
    sysmon_core_config_t config;
} sysmon_context_t;

int sysmon_context_init(sysmon_context_t *context);

#endif
