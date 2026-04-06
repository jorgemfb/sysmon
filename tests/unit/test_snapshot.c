#include "sysmon/alert/evaluator.h"
#include "sysmon/collectors/cpu/cpu.h"
#include "sysmon/collectors/disk/disk.h"
#include "sysmon/collectors/filesystem/filesystem.h"
#include "sysmon/collectors/identity/identity.h"
#include "sysmon/collectors/logs/logs.h"
#include "sysmon/collectors/memory/memory.h"
#include "sysmon/collectors/network/network.h"
#include "sysmon/collectors/package/package.h"
#include "sysmon/collectors/process/process.h"
#include "sysmon/collectors/service/service.h"
#include "sysmon/collectors/smart/smart.h"
#include "sysmon/collectors/temperature/temperature.h"
#include "sysmon/core/context.h"
#include "sysmon/core/snapshot.h"
#include "sysmon/modules/trend/trend_module.h"
#include "sysmon/report/report.h"
#include "sysmon/util/time.h"

#include <stdio.h>
#include <string.h>

static int g_failures = 0;

static void expect_true(int condition, const char *message) {
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", message);
        g_failures++;
    }
}

int main(void) {
    sysmon_snapshot_t alert_snapshot;
    sysmon_snapshot_t snapshot;
    sysmon_snapshot_t trend_snapshot;
    sysmon_context_t trend_context;
    sysmon_module_t trend_module;
    size_t i;
    int rc;

    expect_true(sysmon_snapshot_init(NULL) == -1, "snapshot_init(NULL) must fail");

    memset(&snapshot, 0xAB, sizeof(snapshot));
    expect_true(sysmon_snapshot_init(&snapshot) == 0, "snapshot_init must succeed");
    expect_true(snapshot.sample_count == 0, "snapshot sample_count must start at 0");
    expect_true(snapshot.identity_hostname[0] == '\0', "snapshot hostname must start empty");
    expect_true(snapshot.identity_username[0] == '\0', "snapshot username must start empty");
    expect_true(snapshot.identity_kernel[0] == '\0', "snapshot kernel must start empty");
    expect_true(snapshot.identity_uptime_seconds == 0, "snapshot uptime must start at 0");
    expect_true(snapshot.memory_total_kib == 0, "snapshot memory total must start at 0");
    expect_true(snapshot.memory_available_kib == 0, "snapshot memory available must start at 0");
    expect_true(snapshot.memory_used_kib == 0, "snapshot memory used must start at 0");
    expect_true(snapshot.disk_total_kib == 0, "snapshot disk total must start at 0");
    expect_true(snapshot.disk_available_kib == 0, "snapshot disk available must start at 0");
    expect_true(snapshot.disk_used_kib == 0, "snapshot disk used must start at 0");
    expect_true(snapshot.filesystem_count == 0, "snapshot filesystem count must start at 0");
    expect_true(snapshot.network_count == 0, "snapshot network count must start at 0");
    expect_true(snapshot.process_total_count == 0, "snapshot process total must start at 0");
    expect_true(snapshot.process_running_count == 0, "snapshot process running must start at 0");
    expect_true(snapshot.process_sleeping_count == 0, "snapshot process sleeping must start at 0");
    expect_true(snapshot.service_total_count == 0, "snapshot service total must start at 0");
    expect_true(snapshot.service_active_count == 0, "snapshot service active must start at 0");
    expect_true(snapshot.service_failed_count == 0, "snapshot service failed must start at 0");
    expect_true(snapshot.temperature_count == 0, "snapshot temperature count must start at 0");
    expect_true(snapshot.smart_count == 0, "snapshot smart count must start at 0");
    expect_true(snapshot.log_total_entries_considered == 0, "snapshot log total must start at 0");
    expect_true(snapshot.log_warning_count == 0, "snapshot log warnings must start at 0");
    expect_true(snapshot.log_error_count == 0, "snapshot log errors must start at 0");
    expect_true(snapshot.log_count == 0, "snapshot log message count must start at 0");
    expect_true(snapshot.log_source_available == 0, "snapshot log source availability must start at 0");
    expect_true(snapshot.package_total_installed_count == 0, "snapshot package total count must start at 0");
    expect_true(snapshot.package_explicit_installed_count == 0, "snapshot package explicit count must start at 0");
    expect_true(snapshot.package_manager[0] == '\0', "snapshot package manager must start empty");
    expect_true(snapshot.package_source_available == 0, "snapshot package source availability must start at 0");
    expect_true(snapshot.trend_has_previous == 0, "snapshot trend has_previous must start at 0");
    expect_true(snapshot.trend_cpu_usage_delta == 0.0, "snapshot trend cpu delta must start at 0");
    expect_true(snapshot.trend_cpu_usage_direction == 0, "snapshot trend cpu direction must start at 0");
    expect_true(snapshot.trend_memory_used_kib_delta == 0, "snapshot trend memory delta must start at 0");
    expect_true(snapshot.trend_memory_used_direction == 0, "snapshot trend memory direction must start at 0");
    expect_true(snapshot.trend_disk_available_kib_delta == 0, "snapshot trend disk delta must start at 0");
    expect_true(snapshot.trend_disk_available_direction == 0, "snapshot trend disk direction must start at 0");
    expect_true(snapshot.trend_process_total_count_delta == 0, "snapshot trend process total delta must start at 0");
    expect_true(snapshot.trend_process_total_direction == 0, "snapshot trend process total direction must start at 0");
    expect_true(snapshot.trend_process_running_count_delta == 0, "snapshot trend process running delta must start at 0");
    expect_true(snapshot.trend_process_running_direction == 0, "snapshot trend process running direction must start at 0");
    expect_true(snapshot.trend_process_sleeping_count_delta == 0, "snapshot trend process sleeping delta must start at 0");
    expect_true(snapshot.trend_process_sleeping_direction == 0, "snapshot trend process sleeping direction must start at 0");
    expect_true(snapshot.report_markdown[0] == '\0', "snapshot markdown report must start empty");
    expect_true(snapshot.report_markdown_length == 0, "snapshot markdown report length must start at 0");
    expect_true(snapshot.report_markdown_generated == 0, "snapshot markdown report generated flag must start at 0");
    expect_true(snapshot.alert_count == 0, "snapshot alert count must start at 0");
    expect_true(snapshot.alerts_evaluated == 0, "snapshot alerts evaluated flag must start at 0");
    expect_true(snapshot.cpu_usage_percent == 0.0, "snapshot cpu usage must start at 0");

    rc = sysmon_collect_identity(&snapshot);
    expect_true(rc == 0, "identity collector must succeed");
    expect_true(snapshot.sample_count == 1, "identity collector must increment sample_count");
    expect_true(snapshot.identity_hostname[0] != '\0', "identity collector must store hostname");
    expect_true(snapshot.identity_username[0] != '\0', "identity collector must store username");
    expect_true(snapshot.identity_kernel[0] != '\0', "identity collector must store kernel");

    rc = sysmon_collect_memory(&snapshot);
    expect_true(rc == 0, "memory collector must succeed");
    expect_true(snapshot.memory_total_kib > 0, "memory collector must store total memory");
    expect_true(snapshot.memory_available_kib <= snapshot.memory_total_kib, "memory available must be <= total");
    expect_true(snapshot.memory_used_kib == (snapshot.memory_total_kib - snapshot.memory_available_kib),
                "memory used must match total - available");

    rc = sysmon_collect_disk(&snapshot);
    expect_true(rc == 0, "disk collector must succeed");
    expect_true(snapshot.disk_total_kib > 0, "disk collector must store total disk");
    expect_true(snapshot.disk_available_kib <= snapshot.disk_total_kib, "disk available must be <= total");
    expect_true(snapshot.disk_used_kib == (snapshot.disk_total_kib - snapshot.disk_available_kib),
                "disk used must match total - available");

    rc = sysmon_collect_filesystem(&snapshot);
    expect_true(rc == 0, "filesystem collector must succeed");
    expect_true(snapshot.filesystem_count > 0, "filesystem collector must store at least one entry");
    expect_true(snapshot.filesystem_count <= SYSMON_FILESYSTEM_MAX_ENTRIES,
                "filesystem collector count must not exceed max");
    for (i = 0; i < snapshot.filesystem_count; ++i) {
        expect_true(snapshot.filesystem_entries[i].mountpoint[0] != '\0',
                    "filesystem entry mountpoint must be non-empty");
        expect_true(snapshot.filesystem_entries[i].fs_type[0] != '\0',
                    "filesystem entry fs_type must be non-empty");
        expect_true(snapshot.filesystem_entries[i].total_kib > 0,
                    "filesystem entry total space must be > 0");
        expect_true(snapshot.filesystem_entries[i].available_kib <= snapshot.filesystem_entries[i].total_kib,
                    "filesystem entry available must be <= total");
        expect_true(snapshot.filesystem_entries[i].used_kib ==
                        (snapshot.filesystem_entries[i].total_kib - snapshot.filesystem_entries[i].available_kib),
                    "filesystem entry used must match total - available");
    }

    rc = sysmon_collect_network(&snapshot);
    expect_true(rc == 0, "network collector must succeed");
    expect_true(snapshot.network_count > 0, "network collector must store at least one entry");
    expect_true(snapshot.network_count <= SYSMON_NETWORK_MAX_ENTRIES,
                "network collector count must not exceed max");
    for (i = 0; i < snapshot.network_count; ++i) {
        expect_true(snapshot.network_entries[i].interface_name[0] != '\0',
                    "network entry interface name must be non-empty");
        expect_true(snapshot.network_entries[i].oper_state[0] != '\0',
                    "network entry oper state must be non-empty");
    }

    rc = sysmon_collect_process(&snapshot);
    expect_true(rc == 0, "process collector must succeed");
    expect_true(snapshot.process_total_count > 0, "process collector must store total process count");
    expect_true(snapshot.process_running_count <= snapshot.process_total_count,
                "process running count must be <= total");
    expect_true(snapshot.process_sleeping_count <= snapshot.process_total_count,
                "process sleeping count must be <= total");

    rc = sysmon_collect_service(&snapshot);
    expect_true(rc == 0, "service collector must succeed");
    expect_true(snapshot.service_active_count <= snapshot.service_total_count,
                "service active count must be <= total");
    expect_true(snapshot.service_failed_count <= snapshot.service_total_count,
                "service failed count must be <= total");

    rc = sysmon_collect_temperature(&snapshot);
    expect_true(rc == 0, "temperature collector must succeed");
    expect_true(snapshot.temperature_count <= SYSMON_TEMPERATURE_MAX_ENTRIES,
                "temperature count must not exceed max");
    for (i = 0; i < snapshot.temperature_count; ++i) {
        expect_true(snapshot.temperature_entries[i].sensor_name[0] != '\0',
                    "temperature sensor name must be non-empty");
        expect_true(snapshot.temperature_entries[i].temperature_c > -273.15,
                    "temperature must be above absolute zero");
    }

    rc = sysmon_collect_smart(&snapshot);
    expect_true(rc == 0, "smart collector must succeed");
    expect_true(snapshot.smart_count <= SYSMON_SMART_MAX_ENTRIES,
                "smart count must not exceed max");
    for (i = 0; i < snapshot.smart_count; ++i) {
        expect_true(snapshot.smart_entries[i].device_path[0] != '\0',
                    "smart device path must be non-empty");
        expect_true(snapshot.smart_entries[i].smart_supported >= -1 && snapshot.smart_entries[i].smart_supported <= 1,
                    "smart supported must be in [-1, 1]");
        expect_true(snapshot.smart_entries[i].smart_health[0] != '\0',
                    "smart health string must be non-empty");
    }

    rc = sysmon_collect_logs(&snapshot);
    expect_true(rc == 0, "logs collector must succeed");
    expect_true(snapshot.log_source_available == 0 || snapshot.log_source_available == 1,
                "log source availability must be boolean");
    expect_true(snapshot.log_warning_count + snapshot.log_error_count <= snapshot.log_total_entries_considered,
                "log warnings+errors must be <= total considered");
    expect_true(snapshot.log_count <= SYSMON_LOG_MAX_MESSAGES,
                "log message count must not exceed max");
    for (i = 0; i < snapshot.log_count; ++i) {
        expect_true(snapshot.log_entries[i].level[0] != '\0',
                    "log entry level must be non-empty");
        expect_true(snapshot.log_entries[i].message[0] != '\0',
                    "log entry message must be non-empty");
    }

    rc = sysmon_collect_package(&snapshot);
    expect_true(rc == 0, "package collector must succeed");
    expect_true(snapshot.package_source_available == 0 || snapshot.package_source_available == 1,
                "package source availability must be boolean");
    expect_true(snapshot.package_manager[0] != '\0',
                "package manager identifier must be non-empty");
    expect_true(snapshot.package_explicit_installed_count <= snapshot.package_total_installed_count,
                "package explicit count must be <= package total count");
    if (snapshot.package_source_available == 1) {
        expect_true(strcmp(snapshot.package_manager, "pacman") == 0,
                    "package manager must be pacman when source is available");
    }

    rc = sysmon_collect_cpu(&snapshot);
    expect_true(rc == 0, "cpu collector first run must succeed");
    expect_true(snapshot.cpu_usage_percent >= 0.0 && snapshot.cpu_usage_percent <= 100.0,
                "cpu usage must be in [0, 100] on first run");
    expect_true(snapshot.cpu_usage_percent == 0.0, "cpu usage first run must be 0 due missing delta baseline");

    expect_true(sysmon_time_sleep_ms(20) == 0, "sleep between cpu samples must succeed");
    rc = sysmon_collect_cpu(&snapshot);
    expect_true(rc == 0, "cpu collector second run must succeed");
    expect_true(snapshot.cpu_usage_percent >= 0.0 && snapshot.cpu_usage_percent <= 100.0,
                "cpu usage must be in [0, 100] on second run");

    expect_true(sysmon_context_init(&trend_context) == 0, "trend context init must succeed");
    expect_true(sysmon_snapshot_init(&trend_snapshot) == 0, "trend snapshot init must succeed");
    trend_module = sysmon_trend_module_create();
    expect_true(trend_module.init != NULL, "trend module init callback must exist");
    expect_true(trend_module.collect != NULL, "trend module collect callback must exist");
    expect_true(trend_module.init(&trend_module, &trend_context) == 0, "trend module init must succeed");

    trend_snapshot.cpu_usage_percent = 10.0;
    trend_snapshot.memory_used_kib = 100;
    trend_snapshot.disk_available_kib = 1000;
    trend_snapshot.process_total_count = 50;
    trend_snapshot.process_running_count = 5;
    trend_snapshot.process_sleeping_count = 40;
    expect_true(trend_module.collect(&trend_module, &trend_snapshot) == 0, "trend first collect must succeed");
    expect_true(trend_snapshot.trend_has_previous == 0, "trend first collect must not have previous baseline");
    expect_true(trend_snapshot.trend_cpu_usage_delta == 0.0, "trend first collect cpu delta must be 0");
    expect_true(trend_snapshot.trend_memory_used_kib_delta == 0, "trend first collect memory delta must be 0");
    expect_true(trend_snapshot.trend_disk_available_kib_delta == 0, "trend first collect disk delta must be 0");
    expect_true(trend_snapshot.trend_process_total_count_delta == 0, "trend first collect process total delta must be 0");

    trend_snapshot.cpu_usage_percent = 20.0;
    trend_snapshot.memory_used_kib = 120;
    trend_snapshot.disk_available_kib = 900;
    trend_snapshot.process_total_count = 55;
    trend_snapshot.process_running_count = 4;
    trend_snapshot.process_sleeping_count = 42;
    expect_true(trend_module.collect(&trend_module, &trend_snapshot) == 0, "trend second collect must succeed");
    expect_true(trend_snapshot.trend_has_previous == 1, "trend second collect must use previous baseline");
    expect_true(trend_snapshot.trend_cpu_usage_delta > 9.9 && trend_snapshot.trend_cpu_usage_delta < 10.1,
                "trend cpu delta must be approximately +10");
    expect_true(trend_snapshot.trend_cpu_usage_direction == 1, "trend cpu direction must be increased");
    expect_true(trend_snapshot.trend_memory_used_kib_delta == 20, "trend memory delta must be +20");
    expect_true(trend_snapshot.trend_memory_used_direction == 1, "trend memory direction must be increased");
    expect_true(trend_snapshot.trend_disk_available_kib_delta == -100, "trend disk available delta must be -100");
    expect_true(trend_snapshot.trend_disk_available_direction == -1, "trend disk available direction must be decreased");
    expect_true(trend_snapshot.trend_process_total_count_delta == 5, "trend process total delta must be +5");
    expect_true(trend_snapshot.trend_process_total_direction == 1, "trend process total direction must be increased");
    expect_true(trend_snapshot.trend_process_running_count_delta == -1, "trend process running delta must be -1");
    expect_true(trend_snapshot.trend_process_running_direction == -1, "trend process running direction must be decreased");
    expect_true(trend_snapshot.trend_process_sleeping_count_delta == 2, "trend process sleeping delta must be +2");
    expect_true(trend_snapshot.trend_process_sleeping_direction == 1, "trend process sleeping direction must be increased");

    if (trend_module.shutdown != NULL) {
        trend_module.shutdown(&trend_module);
    }

    expect_true(sysmon_snapshot_init(&alert_snapshot) == 0, "alert snapshot init must succeed");
    alert_snapshot.memory_total_kib = 1000;
    alert_snapshot.memory_used_kib = 960;
    alert_snapshot.cpu_usage_percent = 96.0;
    alert_snapshot.disk_total_kib = 1000;
    alert_snapshot.disk_available_kib = 40;
    alert_snapshot.service_total_count = 20;
    alert_snapshot.service_failed_count = 2;
    alert_snapshot.temperature_count = 1;
    snprintf(alert_snapshot.temperature_entries[0].sensor_name,
             sizeof(alert_snapshot.temperature_entries[0].sensor_name),
             "cpu");
    alert_snapshot.temperature_entries[0].temperature_c = 92.0;
    alert_snapshot.smart_count = 1;
    snprintf(alert_snapshot.smart_entries[0].device_path,
             sizeof(alert_snapshot.smart_entries[0].device_path),
             "/dev/sda");
    alert_snapshot.smart_entries[0].smart_supported = 1;
    snprintf(alert_snapshot.smart_entries[0].smart_health,
             sizeof(alert_snapshot.smart_entries[0].smart_health),
             "failed");

    rc = sysmon_alert_evaluate(&alert_snapshot);
    expect_true(rc == 0, "alert evaluation must succeed");
    expect_true(alert_snapshot.alerts_evaluated == 1, "alerts must be marked as evaluated");
    expect_true(alert_snapshot.alert_count == 6, "all initial alert rules should trigger for crafted snapshot");
    expect_true(alert_snapshot.alert_entries[0].severity[0] != '\0', "first alert severity must be populated");
    expect_true(alert_snapshot.alert_entries[0].title[0] != '\0', "first alert title must be populated");
    expect_true(alert_snapshot.alert_entries[0].message[0] != '\0', "first alert message must be populated");

    snapshot.trend_has_previous = 1;
    snapshot.trend_cpu_usage_delta = 1.5;
    snapshot.trend_cpu_usage_direction = 1;
    snapshot.trend_memory_used_kib_delta = -256;
    snapshot.trend_memory_used_direction = -1;
    snapshot.trend_disk_available_kib_delta = 128;
    snapshot.trend_disk_available_direction = 1;
    snapshot.trend_process_total_count_delta = 2;
    snapshot.trend_process_total_direction = 1;
    snapshot.trend_process_running_count_delta = -1;
    snapshot.trend_process_running_direction = -1;
    snapshot.trend_process_sleeping_count_delta = 1;
    snapshot.trend_process_sleeping_direction = 1;

    rc = sysmon_report_generate(&snapshot);
    expect_true(rc == 0, "report generation must succeed");
    expect_true(snapshot.report_markdown_generated == 1, "report generated flag must be set");
    expect_true(snapshot.report_markdown_length > 0, "report markdown length must be > 0");
    expect_true(strstr(snapshot.report_markdown, "# Sysmon Report") != NULL, "report title must exist");
    expect_true(strstr(snapshot.report_markdown, "Generated:") != NULL, "report timestamp line must exist");
    expect_true(strstr(snapshot.report_markdown, "## Identity Summary") != NULL, "identity section must exist");
    expect_true(strstr(snapshot.report_markdown, "## Memory Summary") != NULL, "memory section must exist");
    expect_true(strstr(snapshot.report_markdown, "## CPU Summary") != NULL, "cpu section must exist");
    expect_true(strstr(snapshot.report_markdown, "## Disk Summary") != NULL, "disk section must exist");
    expect_true(strstr(snapshot.report_markdown, "## Filesystem Summary") != NULL, "filesystem section must exist");
    expect_true(strstr(snapshot.report_markdown, "## Network Summary") != NULL, "network section must exist");
    expect_true(strstr(snapshot.report_markdown, "## Process Summary") != NULL, "process section must exist");
    expect_true(strstr(snapshot.report_markdown, "## Service Summary") != NULL, "service section must exist");
    expect_true(strstr(snapshot.report_markdown, "## Temperature Summary") != NULL, "temperature section must exist");
    expect_true(strstr(snapshot.report_markdown, "## SMART Summary") != NULL, "smart section must exist");
    expect_true(strstr(snapshot.report_markdown, "## Logs Summary") != NULL, "logs section must exist");
    expect_true(strstr(snapshot.report_markdown, "## Package Summary") != NULL, "package section must exist");
    expect_true(strstr(snapshot.report_markdown, "## Trend Summary") != NULL, "trend section must exist");
    expect_true(strstr(snapshot.report_markdown, "## Alerts Summary") != NULL, "alerts section must exist");

    if (g_failures != 0) {
        fprintf(stderr, "test_snapshot: %d failure(s)\n", g_failures);
        return 1;
    }

    printf("test_snapshot: ok\n");
    return 0;
}
