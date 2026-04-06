#ifndef INCLUDE_SYSMON_ALERT_RULE_H
#define INCLUDE_SYSMON_ALERT_RULE_H

#include <stddef.h>

#include "sysmon/core/snapshot.h"

int sysmon_alert_rule_high_memory(const sysmon_snapshot_t *snapshot, const char **severity, const char **title, char *message, size_t message_size);
int sysmon_alert_rule_high_cpu(const sysmon_snapshot_t *snapshot, const char **severity, const char **title, char *message, size_t message_size);
int sysmon_alert_rule_low_disk_available(const sysmon_snapshot_t *snapshot, const char **severity, const char **title, char *message, size_t message_size);
int sysmon_alert_rule_failed_services(const sysmon_snapshot_t *snapshot, const char **severity, const char **title, char *message, size_t message_size);
int sysmon_alert_rule_high_temperature(const sysmon_snapshot_t *snapshot, const char **severity, const char **title, char *message, size_t message_size);
int sysmon_alert_rule_smart_unhealthy(const sysmon_snapshot_t *snapshot, const char **severity, const char **title, char *message, size_t message_size);

#endif
