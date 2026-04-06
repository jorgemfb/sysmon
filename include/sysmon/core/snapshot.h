#ifndef SYSMON_CORE_SNAPSHOT_H
#define SYSMON_CORE_SNAPSHOT_H

#include <stddef.h>
#include <stdint.h>

#define SYSMON_FILESYSTEM_MAX_ENTRIES 64
#define SYSMON_NETWORK_MAX_ENTRIES 64
#define SYSMON_TEMPERATURE_MAX_ENTRIES 64
#define SYSMON_SMART_MAX_ENTRIES 64
#define SYSMON_LOG_MAX_MESSAGES 8
#define SYSMON_ALERT_MAX_ENTRIES 16
#define SYSMON_REPORT_MAX_LENGTH 32768

typedef struct sysmon_filesystem_entry {
    char mountpoint[256];
    char fs_type[64];
    uint64_t total_kib;
    uint64_t available_kib;
    uint64_t used_kib;
} sysmon_filesystem_entry_t;

typedef struct sysmon_network_entry {
    char interface_name[64];
    uint64_t rx_bytes;
    uint64_t tx_bytes;
    char oper_state[32];
} sysmon_network_entry_t;

typedef struct sysmon_temperature_entry {
    char sensor_name[64];
    double temperature_c;
} sysmon_temperature_entry_t;

typedef struct sysmon_smart_entry {
    char device_path[64];
    int smart_supported;
    char smart_health[32];
} sysmon_smart_entry_t;

typedef struct sysmon_log_entry {
    char level[16];
    char message[256];
} sysmon_log_entry_t;

typedef struct sysmon_alert_entry {
    char severity[16];
    char title[64];
    char message[256];
} sysmon_alert_entry_t;

typedef struct sysmon_snapshot {
    char identity_hostname[256];
    char identity_username[256];
    char identity_kernel[256];
    uint64_t identity_uptime_seconds;
    uint64_t memory_total_kib;
    uint64_t memory_available_kib;
    uint64_t memory_used_kib;
    uint64_t disk_total_kib;
    uint64_t disk_available_kib;
    uint64_t disk_used_kib;
    sysmon_filesystem_entry_t filesystem_entries[SYSMON_FILESYSTEM_MAX_ENTRIES];
    size_t filesystem_count;
    sysmon_network_entry_t network_entries[SYSMON_NETWORK_MAX_ENTRIES];
    size_t network_count;
    sysmon_temperature_entry_t temperature_entries[SYSMON_TEMPERATURE_MAX_ENTRIES];
    size_t temperature_count;
    sysmon_smart_entry_t smart_entries[SYSMON_SMART_MAX_ENTRIES];
    size_t smart_count;
    uint64_t log_total_entries_considered;
    uint64_t log_warning_count;
    uint64_t log_error_count;
    sysmon_log_entry_t log_entries[SYSMON_LOG_MAX_MESSAGES];
    size_t log_count;
    int log_source_available;
    uint64_t package_total_installed_count;
    uint64_t package_explicit_installed_count;
    char package_manager[32];
    int package_source_available;
    int trend_has_previous;
    double trend_cpu_usage_delta;
    int trend_cpu_usage_direction;
    int64_t trend_memory_used_kib_delta;
    int trend_memory_used_direction;
    int64_t trend_disk_available_kib_delta;
    int trend_disk_available_direction;
    int64_t trend_process_total_count_delta;
    int trend_process_total_direction;
    int64_t trend_process_running_count_delta;
    int trend_process_running_direction;
    int64_t trend_process_sleeping_count_delta;
    int trend_process_sleeping_direction;
    sysmon_alert_entry_t alert_entries[SYSMON_ALERT_MAX_ENTRIES];
    size_t alert_count;
    int alerts_evaluated;
    char report_markdown[SYSMON_REPORT_MAX_LENGTH];
    size_t report_markdown_length;
    int report_markdown_generated;
    uint64_t process_total_count;
    uint64_t process_running_count;
    uint64_t process_sleeping_count;
    uint64_t service_total_count;
    uint64_t service_active_count;
    uint64_t service_failed_count;
    double cpu_usage_percent;
    size_t sample_count;
} sysmon_snapshot_t;

int sysmon_snapshot_init(sysmon_snapshot_t *snapshot);

#endif
