#include "sysmon/core/context.h"

#include <stddef.h>

int sysmon_context_init(sysmon_context_t *context) {
    if (context == NULL) {
        return -1;
    }

    return sysmon_core_config_init(&context->config);
}
