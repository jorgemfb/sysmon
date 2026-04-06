#include "sysmon/config/env.h"

#include <stddef.h>
#include <stdlib.h>

const char *sysmon_env_get(const char *name) {
    if (name == NULL) {
        return NULL;
    }

    return getenv(name);
}
