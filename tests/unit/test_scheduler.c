#include "sysmon/core/context.h"
#include "sysmon/core/module.h"
#include "sysmon/core/registry.h"
#include "sysmon/core/scheduler.h"
#include "sysmon/core/snapshot.h"
#include "sysmon/modules/cpu/cpu_module.h"
#include "sysmon/modules/disk/disk_module.h"
#include "sysmon/modules/filesystem/filesystem_module.h"
#include "sysmon/modules/identity/identity_module.h"
#include "sysmon/modules/log/log_module.h"
#include "sysmon/modules/memory/memory_module.h"
#include "sysmon/modules/network/network_module.h"
#include "sysmon/modules/package/package_module.h"
#include "sysmon/modules/process/process_module.h"
#include "sysmon/modules/report/report_module.h"
#include "sysmon/modules/service/service_module.h"
#include "sysmon/modules/smart/smart_module.h"
#include "sysmon/modules/temperature/temperature_module.h"
#include "sysmon/modules/trend/trend_module.h"
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

static int counter_collect(sysmon_module_t *module, struct sysmon_snapshot *snapshot) {
    int *counter;

    if (module == NULL || snapshot == NULL || module->userdata == NULL) {
        return -1;
    }

    counter = (int *)module->userdata;
    (*counter)++;
    snapshot->sample_count++;
    return 0;
}

int main(void) {
    sysmon_scheduler_t scheduler;
    sysmon_registry_t registry;
    sysmon_snapshot_t snapshot;
    sysmon_module_t module;
    int counter = 0;

    sysmon_context_t context;
    sysmon_registry_t id_registry;
    sysmon_snapshot_t id_snapshot;
    sysmon_module_t cpu_module;
    sysmon_module_t disk_module;
    sysmon_module_t filesystem_module;
    sysmon_module_t identity_module;
    sysmon_module_t log_module;
    sysmon_module_t memory_module;
    sysmon_module_t network_module;
    sysmon_module_t package_module;
    sysmon_module_t process_module;
    sysmon_module_t report_module;
    sysmon_module_t service_module;
    sysmon_module_t smart_module;
    sysmon_module_t temperature_module;
    sysmon_module_t trend_module;

    expect_true(sysmon_scheduler_init(NULL) == -1, "scheduler_init(NULL) must fail");
    expect_true(sysmon_scheduler_init(&scheduler) == 0, "scheduler_init must succeed");
    expect_true(sysmon_registry_init(&registry) == 0, "registry_init must succeed");
    expect_true(sysmon_snapshot_init(&snapshot) == 0, "snapshot_init must succeed");

    module.name = "counter";
    module.interval_ms = 10;
    module.userdata = &counter;
    module.init = NULL;
    module.collect = counter_collect;
    module.shutdown = NULL;

    expect_true(sysmon_registry_add(&registry, &module) == 0, "registry_add(counter) must succeed");

    expect_true(sysmon_scheduler_run_once(&scheduler, &registry, &snapshot) == 0, "first scheduler run must succeed");
    expect_true(counter == 1, "first scheduler run must collect");
    expect_true(snapshot.sample_count == 1, "first scheduler run must update snapshot");

    expect_true(sysmon_scheduler_run_once(&scheduler, &registry, &snapshot) == 0, "second scheduler run must succeed");
    expect_true(counter == 1, "second immediate run must be gated by interval");

    expect_true(sysmon_time_sleep_ms(15) == 0, "sleep before third run must succeed");
    expect_true(sysmon_scheduler_run_once(&scheduler, &registry, &snapshot) == 0, "third scheduler run must succeed");
    expect_true(counter == 2, "third run after interval must collect again");
    expect_true(snapshot.sample_count == 2, "third run must update snapshot again");

    scheduler.running = false;
    expect_true(sysmon_scheduler_run_once(&scheduler, &registry, &snapshot) == 0, "run_once on stopped scheduler must succeed");
    expect_true(counter == 2, "stopped scheduler must not collect");

    expect_true(sysmon_scheduler_init(&scheduler) == 0, "scheduler re-init must succeed");
    expect_true(sysmon_registry_init(&id_registry) == 0, "identity registry init must succeed");
    expect_true(sysmon_snapshot_init(&id_snapshot) == 0, "identity snapshot init must succeed");
    expect_true(sysmon_context_init(&context) == 0, "context init must succeed");

    identity_module = sysmon_identity_module_create();
    expect_true(identity_module.init != NULL, "identity module init callback must exist");
    expect_true(identity_module.init(&identity_module, &context) == 0, "identity module init must succeed");
    expect_true(sysmon_registry_add(&id_registry, &identity_module) == 0, "identity module registry add must succeed");

    memory_module = sysmon_memory_module_create();
    expect_true(memory_module.init != NULL, "memory module init callback must exist");
    expect_true(memory_module.init(&memory_module, &context) == 0, "memory module init must succeed");
    expect_true(sysmon_registry_add(&id_registry, &memory_module) == 0, "memory module registry add must succeed");

    cpu_module = sysmon_cpu_module_create();
    expect_true(cpu_module.init != NULL, "cpu module init callback must exist");
    expect_true(cpu_module.init(&cpu_module, &context) == 0, "cpu module init must succeed");
    expect_true(sysmon_registry_add(&id_registry, &cpu_module) == 0, "cpu module registry add must succeed");

    disk_module = sysmon_disk_module_create();
    expect_true(disk_module.init != NULL, "disk module init callback must exist");
    expect_true(disk_module.init(&disk_module, &context) == 0, "disk module init must succeed");
    expect_true(sysmon_registry_add(&id_registry, &disk_module) == 0, "disk module registry add must succeed");

    filesystem_module = sysmon_filesystem_module_create();
    expect_true(filesystem_module.init != NULL, "filesystem module init callback must exist");
    expect_true(filesystem_module.init(&filesystem_module, &context) == 0, "filesystem module init must succeed");
    expect_true(sysmon_registry_add(&id_registry, &filesystem_module) == 0, "filesystem module registry add must succeed");

    network_module = sysmon_network_module_create();
    expect_true(network_module.init != NULL, "network module init callback must exist");
    expect_true(network_module.init(&network_module, &context) == 0, "network module init must succeed");
    expect_true(sysmon_registry_add(&id_registry, &network_module) == 0, "network module registry add must succeed");

    process_module = sysmon_process_module_create();
    expect_true(process_module.init != NULL, "process module init callback must exist");
    expect_true(process_module.init(&process_module, &context) == 0, "process module init must succeed");
    expect_true(sysmon_registry_add(&id_registry, &process_module) == 0, "process module registry add must succeed");

    service_module = sysmon_service_module_create();
    expect_true(service_module.init != NULL, "service module init callback must exist");
    expect_true(service_module.init(&service_module, &context) == 0, "service module init must succeed");
    expect_true(sysmon_registry_add(&id_registry, &service_module) == 0, "service module registry add must succeed");

    temperature_module = sysmon_temperature_module_create();
    expect_true(temperature_module.init != NULL, "temperature module init callback must exist");
    expect_true(temperature_module.init(&temperature_module, &context) == 0, "temperature module init must succeed");
    expect_true(sysmon_registry_add(&id_registry, &temperature_module) == 0, "temperature module registry add must succeed");

    smart_module = sysmon_smart_module_create();
    expect_true(smart_module.init != NULL, "smart module init callback must exist");
    expect_true(smart_module.init(&smart_module, &context) == 0, "smart module init must succeed");
    expect_true(sysmon_registry_add(&id_registry, &smart_module) == 0, "smart module registry add must succeed");

    log_module = sysmon_log_module_create();
    expect_true(log_module.init != NULL, "log module init callback must exist");
    expect_true(log_module.init(&log_module, &context) == 0, "log module init must succeed");
    expect_true(sysmon_registry_add(&id_registry, &log_module) == 0, "log module registry add must succeed");

    package_module = sysmon_package_module_create();
    expect_true(package_module.init != NULL, "package module init callback must exist");
    expect_true(package_module.init(&package_module, &context) == 0, "package module init must succeed");
    expect_true(sysmon_registry_add(&id_registry, &package_module) == 0, "package module registry add must succeed");

    trend_module = sysmon_trend_module_create();
    expect_true(trend_module.init != NULL, "trend module init callback must exist");
    expect_true(trend_module.init(&trend_module, &context) == 0, "trend module init must succeed");
    trend_module.interval_ms = 1;
    expect_true(sysmon_registry_add(&id_registry, &trend_module) == 0, "trend module registry add must succeed");

    report_module = sysmon_report_module_create();
    expect_true(report_module.init != NULL, "report module init callback must exist");
    expect_true(report_module.init(&report_module, &context) == 0, "report module init must succeed");
    report_module.interval_ms = 1;
    expect_true(sysmon_registry_add(&id_registry, &report_module) == 0, "report module registry add must succeed");

    expect_true(sysmon_scheduler_run_once(&scheduler, &id_registry, &id_snapshot) == 0, "identity scheduler run must succeed");
    expect_true(id_snapshot.sample_count >= 1, "scheduler run must increment sample count");
    expect_true(id_snapshot.identity_hostname[0] != '\0', "identity hostname must be populated");
    expect_true(id_snapshot.identity_username[0] != '\0', "identity username must be populated");
    expect_true(id_snapshot.identity_kernel[0] != '\0', "identity kernel must be populated");
    expect_true(id_snapshot.memory_total_kib > 0, "memory total must be populated through scheduler");
    expect_true(id_snapshot.memory_available_kib <= id_snapshot.memory_total_kib,
                "memory available must be <= total through scheduler");
    expect_true(id_snapshot.memory_used_kib == (id_snapshot.memory_total_kib - id_snapshot.memory_available_kib),
                "memory used must match total - available through scheduler");
    expect_true(id_snapshot.disk_total_kib > 0, "disk total must be populated through scheduler");
    expect_true(id_snapshot.disk_available_kib <= id_snapshot.disk_total_kib,
                "disk available must be <= total through scheduler");
    expect_true(id_snapshot.disk_used_kib == (id_snapshot.disk_total_kib - id_snapshot.disk_available_kib),
                "disk used must match total - available through scheduler");
    expect_true(id_snapshot.filesystem_count > 0, "filesystem entries must be populated through scheduler");
    expect_true(id_snapshot.filesystem_count <= SYSMON_FILESYSTEM_MAX_ENTRIES,
                "filesystem count through scheduler must not exceed max");
    expect_true(id_snapshot.filesystem_entries[0].mountpoint[0] != '\0',
                "first filesystem mountpoint through scheduler must be populated");
    expect_true(id_snapshot.filesystem_entries[0].fs_type[0] != '\0',
                "first filesystem type through scheduler must be populated");
    expect_true(id_snapshot.network_count > 0, "network entries must be populated through scheduler");
    expect_true(id_snapshot.network_count <= SYSMON_NETWORK_MAX_ENTRIES,
                "network count through scheduler must not exceed max");
    expect_true(id_snapshot.network_entries[0].interface_name[0] != '\0',
                "first network interface through scheduler must be populated");
    expect_true(id_snapshot.network_entries[0].oper_state[0] != '\0',
                "first network operstate through scheduler must be populated");
    expect_true(id_snapshot.process_total_count > 0,
                "process total through scheduler must be populated");
    expect_true(id_snapshot.process_running_count <= id_snapshot.process_total_count,
                "process running through scheduler must be <= total");
    expect_true(id_snapshot.process_sleeping_count <= id_snapshot.process_total_count,
                "process sleeping through scheduler must be <= total");
    expect_true(id_snapshot.service_active_count <= id_snapshot.service_total_count,
                "service active through scheduler must be <= total");
    expect_true(id_snapshot.service_failed_count <= id_snapshot.service_total_count,
                "service failed through scheduler must be <= total");
    expect_true(id_snapshot.temperature_count <= SYSMON_TEMPERATURE_MAX_ENTRIES,
                "temperature count through scheduler must not exceed max");
    if (id_snapshot.temperature_count > 0) {
        expect_true(id_snapshot.temperature_entries[0].sensor_name[0] != '\0',
                    "first temperature sensor through scheduler must be populated");
    }
    expect_true(id_snapshot.smart_count <= SYSMON_SMART_MAX_ENTRIES,
                "smart count through scheduler must not exceed max");
    if (id_snapshot.smart_count > 0) {
        expect_true(id_snapshot.smart_entries[0].device_path[0] != '\0',
                    "first smart device through scheduler must be populated");
        expect_true(id_snapshot.smart_entries[0].smart_health[0] != '\0',
                    "first smart health through scheduler must be populated");
    }
    expect_true(id_snapshot.log_source_available == 0 || id_snapshot.log_source_available == 1,
                "log source availability through scheduler must be boolean");
    expect_true(id_snapshot.log_warning_count + id_snapshot.log_error_count <= id_snapshot.log_total_entries_considered,
                "log warning+error through scheduler must be <= total considered");
    expect_true(id_snapshot.log_count <= SYSMON_LOG_MAX_MESSAGES,
                "log message count through scheduler must not exceed max");
    if (id_snapshot.log_count > 0) {
        expect_true(id_snapshot.log_entries[0].level[0] != '\0',
                    "first log level through scheduler must be populated");
        expect_true(id_snapshot.log_entries[0].message[0] != '\0',
                    "first log message through scheduler must be populated");
    }
    expect_true(id_snapshot.package_source_available == 0 || id_snapshot.package_source_available == 1,
                "package source availability through scheduler must be boolean");
    expect_true(id_snapshot.package_manager[0] != '\0',
                "package manager through scheduler must be non-empty");
    expect_true(id_snapshot.package_explicit_installed_count <= id_snapshot.package_total_installed_count,
                "package explicit through scheduler must be <= total");
    if (id_snapshot.package_source_available == 1) {
        expect_true(strcmp(id_snapshot.package_manager, "pacman") == 0,
                    "package manager through scheduler must be pacman when source is available");
    }
    expect_true(id_snapshot.trend_has_previous == 0,
                "trend should not have previous baseline on first scheduler run");
    expect_true(id_snapshot.report_markdown_generated == 1,
                "report should be generated on first scheduler run");
    expect_true(id_snapshot.report_markdown_length > 0,
                "report markdown length on first scheduler run must be > 0");
    expect_true(id_snapshot.alerts_evaluated == 1,
                "alerts should be evaluated on first scheduler run");
    expect_true(strstr(id_snapshot.report_markdown, "## Identity Summary") != NULL,
                "report must include identity section through scheduler");
    expect_true(strstr(id_snapshot.report_markdown, "## Alerts Summary") != NULL,
                "report must include alerts section through scheduler");
    expect_true(id_snapshot.cpu_usage_percent >= 0.0 && id_snapshot.cpu_usage_percent <= 100.0,
                "cpu usage must be in [0, 100] on first scheduler sample");

    expect_true(sysmon_time_sleep_ms(20) == 0, "sleep between cpu scheduler samples must succeed");
    expect_true(sysmon_scheduler_run_once(&scheduler, &id_registry, &id_snapshot) == 0,
                "second scheduler run with cpu module must succeed");
    expect_true(id_snapshot.cpu_usage_percent >= 0.0 && id_snapshot.cpu_usage_percent <= 100.0,
                "cpu usage must be in [0, 100] on second scheduler sample");
    expect_true(id_snapshot.trend_has_previous == 1,
                "trend should have previous baseline on second scheduler run");
    expect_true(id_snapshot.trend_cpu_usage_direction >= -1 && id_snapshot.trend_cpu_usage_direction <= 1,
                "trend cpu direction through scheduler must be in [-1, 1]");
    expect_true(id_snapshot.trend_memory_used_direction >= -1 && id_snapshot.trend_memory_used_direction <= 1,
                "trend memory direction through scheduler must be in [-1, 1]");
    expect_true(id_snapshot.trend_disk_available_direction >= -1 && id_snapshot.trend_disk_available_direction <= 1,
                "trend disk direction through scheduler must be in [-1, 1]");
    expect_true(id_snapshot.trend_process_total_direction >= -1 && id_snapshot.trend_process_total_direction <= 1,
                "trend process total direction through scheduler must be in [-1, 1]");
    expect_true(id_snapshot.trend_process_running_direction >= -1 && id_snapshot.trend_process_running_direction <= 1,
                "trend process running direction through scheduler must be in [-1, 1]");
    expect_true(id_snapshot.trend_process_sleeping_direction >= -1 && id_snapshot.trend_process_sleeping_direction <= 1,
                "trend process sleeping direction through scheduler must be in [-1, 1]");
    expect_true(id_snapshot.report_markdown_generated == 1,
                "report should be generated on second scheduler run");
    expect_true(strstr(id_snapshot.report_markdown, "## Trend Summary") != NULL,
                "report must include trend section through scheduler");
    expect_true(id_snapshot.alerts_evaluated == 1,
                "alerts should be evaluated on second scheduler run");

    if (g_failures != 0) {
        fprintf(stderr, "test_scheduler: %d failure(s)\n", g_failures);
        return 1;
    }

    printf("test_scheduler: ok\n");
    return 0;
}
