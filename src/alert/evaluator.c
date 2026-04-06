#include "sysmon/alert/evaluator.h"

#include "sysmon/alert/alert.h"
#include "sysmon/alert/rule.h"

#include <stddef.h>

typedef int (*sysmon_alert_rule_fn)(const sysmon_snapshot_t *snapshot,
                                    const char **severity,
                                    const char **title,
                                    char *message,
                                    size_t message_size);

int sysmon_alert_evaluate(sysmon_snapshot_t *snapshot) {
    static const sysmon_alert_rule_fn rules[] = {
        sysmon_alert_rule_high_memory,
        sysmon_alert_rule_high_cpu,
        sysmon_alert_rule_low_disk_available,
        sysmon_alert_rule_failed_services,
        sysmon_alert_rule_high_temperature,
        sysmon_alert_rule_smart_unhealthy,
    };
    size_t i;

    if (snapshot == NULL) {
        return -1;
    }

    if (sysmon_alert_reset(snapshot) != 0) {
        return -1;
    }

    for (i = 0; i < (sizeof(rules) / sizeof(rules[0])); ++i) {
        const char *severity = NULL;
        const char *title = NULL;
        char message[256];
        int rc;

        message[0] = '\0';
        rc = rules[i](snapshot, &severity, &title, message, sizeof(message));
        if (rc < 0) {
            return -2;
        }

        if (rc == 1 && severity != NULL && title != NULL && message[0] != '\0') {
            int add_rc = sysmon_alert_add(snapshot, severity, title, message);
            if (add_rc == -2) {
                break;
            }
            if (add_rc != 0) {
                return -3;
            }
        }
    }

    snapshot->alerts_evaluated = 1;
    return 0;
}
