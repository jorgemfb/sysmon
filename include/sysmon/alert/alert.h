#ifndef INCLUDE_SYSMON_ALERT_ALERT_H
#define INCLUDE_SYSMON_ALERT_ALERT_H

#include "sysmon/core/snapshot.h"

int sysmon_alert_reset(sysmon_snapshot_t *snapshot);
int sysmon_alert_add(sysmon_snapshot_t *snapshot, const char *severity, const char *title, const char *message);

#endif
