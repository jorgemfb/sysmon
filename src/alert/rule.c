#include "sysmon/alert/rule.h"

#include <stdio.h>
#include <string.h>
#include <strings.h>

int sysmon_alert_rule_high_memory(const sysmon_snapshot_t *snapshot, const char **severity, const char **title, char *message, size_t message_size) {
    double usage_pct;

    if (snapshot == NULL || severity == NULL || title == NULL || message == NULL || message_size == 0) {
        return -1;
    }

    if (snapshot->memory_total_kib == 0) {
        return 0;
    }

    usage_pct = ((double)snapshot->memory_used_kib * 100.0) / (double)snapshot->memory_total_kib;
    if (usage_pct >= 95.0) {
        *severity = "critical";
    } else if (usage_pct >= 85.0) {
        *severity = "warning";
    } else {
        return 0;
    }

    *title = "High Memory Usage";
    (void)snprintf(message,
                   message_size,
                   "Memory usage is %.1f%% (%llu / %llu KiB).",
                   usage_pct,
                   (unsigned long long)snapshot->memory_used_kib,
                   (unsigned long long)snapshot->memory_total_kib);
    return 1;
}

int sysmon_alert_rule_high_cpu(const sysmon_snapshot_t *snapshot, const char **severity, const char **title, char *message, size_t message_size) {
    if (snapshot == NULL || severity == NULL || title == NULL || message == NULL || message_size == 0) {
        return -1;
    }

    if (snapshot->cpu_usage_percent >= 95.0) {
        *severity = "critical";
    } else if (snapshot->cpu_usage_percent >= 85.0) {
        *severity = "warning";
    } else {
        return 0;
    }

    *title = "High CPU Usage";
    (void)snprintf(message, message_size, "CPU usage is %.1f%%.", snapshot->cpu_usage_percent);
    return 1;
}

int sysmon_alert_rule_low_disk_available(const sysmon_snapshot_t *snapshot, const char **severity, const char **title, char *message, size_t message_size) {
    double available_pct;

    if (snapshot == NULL || severity == NULL || title == NULL || message == NULL || message_size == 0) {
        return -1;
    }

    if (snapshot->disk_total_kib == 0) {
        return 0;
    }

    available_pct = ((double)snapshot->disk_available_kib * 100.0) / (double)snapshot->disk_total_kib;
    if (available_pct <= 5.0) {
        *severity = "critical";
    } else if (available_pct <= 10.0) {
        *severity = "warning";
    } else {
        return 0;
    }

    *title = "Low Disk Space";
    (void)snprintf(message,
                   message_size,
                   "Disk '/' available space is %.1f%% (%llu / %llu KiB).",
                   available_pct,
                   (unsigned long long)snapshot->disk_available_kib,
                   (unsigned long long)snapshot->disk_total_kib);
    return 1;
}

int sysmon_alert_rule_failed_services(const sysmon_snapshot_t *snapshot, const char **severity, const char **title, char *message, size_t message_size) {
    if (snapshot == NULL || severity == NULL || title == NULL || message == NULL || message_size == 0) {
        return -1;
    }

    if (snapshot->service_total_count == 0 || snapshot->service_failed_count == 0) {
        return 0;
    }

    if (snapshot->service_failed_count >= 5) {
        *severity = "critical";
    } else {
        *severity = "warning";
    }

    *title = "Failed Services Detected";
    (void)snprintf(message,
                   message_size,
                   "Detected %llu failed services out of %llu.",
                   (unsigned long long)snapshot->service_failed_count,
                   (unsigned long long)snapshot->service_total_count);
    return 1;
}

int sysmon_alert_rule_high_temperature(const sysmon_snapshot_t *snapshot, const char **severity, const char **title, char *message, size_t message_size) {
    size_t i;
    double max_temp = -273.15;
    const char *sensor_name = NULL;

    if (snapshot == NULL || severity == NULL || title == NULL || message == NULL || message_size == 0) {
        return -1;
    }

    if (snapshot->temperature_count == 0) {
        return 0;
    }

    for (i = 0; i < snapshot->temperature_count; ++i) {
        if (snapshot->temperature_entries[i].temperature_c > max_temp) {
            max_temp = snapshot->temperature_entries[i].temperature_c;
            sensor_name = snapshot->temperature_entries[i].sensor_name;
        }
    }

    if (max_temp >= 90.0) {
        *severity = "critical";
    } else if (max_temp >= 80.0) {
        *severity = "warning";
    } else {
        return 0;
    }

    *title = "High Temperature";
    (void)snprintf(message,
                   message_size,
                   "Highest temperature is %.1f C at sensor '%s'.",
                   max_temp,
                   (sensor_name != NULL && sensor_name[0] != '\0') ? sensor_name : "unknown");
    return 1;
}

int sysmon_alert_rule_smart_unhealthy(const sysmon_snapshot_t *snapshot, const char **severity, const char **title, char *message, size_t message_size) {
    size_t i;

    if (snapshot == NULL || severity == NULL || title == NULL || message == NULL || message_size == 0) {
        return -1;
    }

    if (snapshot->smart_count == 0) {
        return 0;
    }

    for (i = 0; i < snapshot->smart_count; ++i) {
        if (snapshot->smart_entries[i].smart_supported == 1 &&
            strcasecmp(snapshot->smart_entries[i].smart_health, "passed") != 0) {
            *severity = "critical";
            *title = "SMART Health Issue";
            (void)snprintf(message,
                           message_size,
                           "Device %s reports SMART health '%s'.",
                           snapshot->smart_entries[i].device_path,
                           snapshot->smart_entries[i].smart_health);
            return 1;
        }
    }

    return 0;
}
