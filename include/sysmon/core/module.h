#ifndef SYSMON_CORE_MODULE_H
#define SYSMON_CORE_MODULE_H

#include <stdint.h>

struct sysmon_context;
struct sysmon_snapshot;

typedef struct sysmon_module sysmon_module_t;

typedef int (*sysmon_module_init_fn)(sysmon_module_t *module, struct sysmon_context *context);
typedef int (*sysmon_module_collect_fn)(sysmon_module_t *module, struct sysmon_snapshot *snapshot);
typedef void (*sysmon_module_shutdown_fn)(sysmon_module_t *module);

struct sysmon_module {
    const char *name;
    uint64_t interval_ms;
    void *userdata;
    sysmon_module_init_fn init;
    sysmon_module_collect_fn collect;
    sysmon_module_shutdown_fn shutdown;
};

int sysmon_module_is_valid(const sysmon_module_t *module);

#endif
