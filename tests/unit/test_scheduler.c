#include "sysmon/core/context.h"
#include "sysmon/core/module.h"
#include "sysmon/core/registry.h"
#include "sysmon/core/scheduler.h"
#include "sysmon/core/snapshot.h"
#include "sysmon/modules/identity/identity_module.h"
#include "sysmon/util/time.h"

#include <stdio.h>

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
    sysmon_module_t identity_module;

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
    expect_true(sysmon_scheduler_run_once(&scheduler, &id_registry, &id_snapshot) == 0, "identity scheduler run must succeed");
    expect_true(id_snapshot.sample_count == 1, "identity collect through scheduler must increment sample count");
    expect_true(id_snapshot.identity_hostname[0] != '\0', "identity hostname must be populated");
    expect_true(id_snapshot.identity_username[0] != '\0', "identity username must be populated");
    expect_true(id_snapshot.identity_kernel[0] != '\0', "identity kernel must be populated");

    if (g_failures != 0) {
        fprintf(stderr, "test_scheduler: %d failure(s)\n", g_failures);
        return 1;
    }

    printf("test_scheduler: ok\n");
    return 0;
}
