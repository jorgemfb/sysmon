#include "sysmon/core/module.h"
#include "sysmon/core/registry.h"
#include "sysmon/core/snapshot.h"

#include <stdio.h>
#include <string.h>

static int g_failures = 0;

static void expect_true(int condition, const char *message) {
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", message);
        g_failures++;
    }
}

static int test_collect(sysmon_module_t *module, struct sysmon_snapshot *snapshot) {
    (void)module;
    if (snapshot != NULL) {
        snapshot->sample_count++;
    }
    return 0;
}

int main(void) {
    sysmon_registry_t registry;
    sysmon_module_t module;
    sysmon_module_t invalid_module;
    sysmon_snapshot_t snapshot;
    sysmon_module_t *stored;
    size_t i;

    expect_true(sysmon_registry_init(NULL) == -1, "registry_init(NULL) must fail");
    expect_true(sysmon_registry_init(&registry) == 0, "registry_init must succeed");
    expect_true(registry.count == 0, "registry count must start at 0");

    module.name = "test";
    module.interval_ms = 1;
    module.userdata = NULL;
    module.init = NULL;
    module.collect = test_collect;
    module.shutdown = NULL;

    expect_true(sysmon_registry_add(&registry, &module) == 0, "registry_add(valid) must succeed");
    expect_true(registry.count == 1, "registry count must be 1 after add");

    stored = sysmon_registry_get(&registry, 0);
    expect_true(stored != NULL, "registry_get(0) must return module");
    expect_true(stored != NULL && strcmp(stored->name, "test") == 0, "stored module name must match");
    expect_true(sysmon_snapshot_init(&snapshot) == 0, "snapshot init must succeed");
    expect_true(stored != NULL && stored->collect(stored, &snapshot) == 0, "stored collect callback must be callable");
    expect_true(snapshot.sample_count == 1, "collect callback must update snapshot");

    invalid_module = module;
    invalid_module.collect = NULL;
    expect_true(sysmon_registry_add(&registry, &invalid_module) == -1, "registry_add(invalid module) must fail");
    expect_true(sysmon_registry_add(NULL, &module) == -1, "registry_add(NULL, module) must fail");
    expect_true(sysmon_registry_get(&registry, 1) == NULL, "registry_get(out of range) must return NULL");

    expect_true(sysmon_registry_init(&registry) == 0, "registry re-init must succeed");
    for (i = 0; i < SYSMON_REGISTRY_MAX_MODULES; ++i) {
        expect_true(sysmon_registry_add(&registry, &module) == 0, "registry must accept up to max modules");
    }
    expect_true(sysmon_registry_add(&registry, &module) == -2, "registry must reject when full");

    if (g_failures != 0) {
        fprintf(stderr, "test_registry: %d failure(s)\n", g_failures);
        return 1;
    }

    printf("test_registry: ok\n");
    return 0;
}
