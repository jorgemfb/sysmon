#include "sysmon/core/snapshot.h"

#include <stddef.h>
#include <string.h>

int sysmon_snapshot_init(sysmon_snapshot_t *snapshot) {
    if (snapshot == NULL) {
        return -1;
    }

    memset(snapshot, 0, sizeof(*snapshot));
    return 0;
}
