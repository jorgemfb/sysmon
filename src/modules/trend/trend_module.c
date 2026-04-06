#include "sysmon/modules/trend/trend_module.h"
#include "sysmon/config/defaults.h"
#include "sysmon/core/snapshot.h"

#include <stdint.h>
#include <string.h>

typedef struct sysmon_trend_state {
    int has_previous;
    sysmon_snapshot_t previous_snapshot;
} sysmon_trend_state_t;

static int sysmon_trend_direction_double(double delta) {
    if (delta > 0.0) {
        return 1;
    }
    if (delta < 0.0) {
        return -1;
    }
    return 0;
}

static int64_t sysmon_trend_delta_u64(uint64_t current, uint64_t previous) {
    if (current >= previous) {
        return (int64_t)(current - previous);
    }
    return -(int64_t)(previous - current);
}

static int sysmon_trend_direction_i64(int64_t delta) {
    if (delta > 0) {
        return 1;
    }
    if (delta < 0) {
        return -1;
    }
    return 0;
}

static int sysmon_trend_init(sysmon_module_t *module, struct sysmon_context *context) {
    sysmon_trend_state_t *state;

    if (module == NULL || context == NULL) {
        return -1;
    }

    module->interval_ms = context->config.default_interval_ms;
    if (module->interval_ms == 0) {
        module->interval_ms = SYSMON_DEFAULT_INTERVAL_MS;
    }

    state = (sysmon_trend_state_t *)module->userdata;
    if (state != NULL) {
        state->has_previous = 0;
        memset(&state->previous_snapshot, 0, sizeof(state->previous_snapshot));
    }

    return 0;
}

static int sysmon_trend_collect(sysmon_module_t *module, struct sysmon_snapshot *snapshot) {
    sysmon_trend_state_t *state;
    int64_t delta_i64;
    double delta_double;

    if (module == NULL || snapshot == NULL || module->userdata == NULL) {
        return -1;
    }

    state = (sysmon_trend_state_t *)module->userdata;

    if (!state->has_previous) {
        snapshot->trend_has_previous = 0;
        snapshot->trend_cpu_usage_delta = 0.0;
        snapshot->trend_cpu_usage_direction = 0;
        snapshot->trend_memory_used_kib_delta = 0;
        snapshot->trend_memory_used_direction = 0;
        snapshot->trend_disk_available_kib_delta = 0;
        snapshot->trend_disk_available_direction = 0;
        snapshot->trend_process_total_count_delta = 0;
        snapshot->trend_process_total_direction = 0;
        snapshot->trend_process_running_count_delta = 0;
        snapshot->trend_process_running_direction = 0;
        snapshot->trend_process_sleeping_count_delta = 0;
        snapshot->trend_process_sleeping_direction = 0;
    } else {
        snapshot->trend_has_previous = 1;

        delta_double = snapshot->cpu_usage_percent - state->previous_snapshot.cpu_usage_percent;
        snapshot->trend_cpu_usage_delta = delta_double;
        snapshot->trend_cpu_usage_direction = sysmon_trend_direction_double(delta_double);

        delta_i64 = sysmon_trend_delta_u64(snapshot->memory_used_kib, state->previous_snapshot.memory_used_kib);
        snapshot->trend_memory_used_kib_delta = delta_i64;
        snapshot->trend_memory_used_direction = sysmon_trend_direction_i64(delta_i64);

        delta_i64 = sysmon_trend_delta_u64(snapshot->disk_available_kib, state->previous_snapshot.disk_available_kib);
        snapshot->trend_disk_available_kib_delta = delta_i64;
        snapshot->trend_disk_available_direction = sysmon_trend_direction_i64(delta_i64);

        delta_i64 = sysmon_trend_delta_u64(snapshot->process_total_count, state->previous_snapshot.process_total_count);
        snapshot->trend_process_total_count_delta = delta_i64;
        snapshot->trend_process_total_direction = sysmon_trend_direction_i64(delta_i64);

        delta_i64 = sysmon_trend_delta_u64(snapshot->process_running_count, state->previous_snapshot.process_running_count);
        snapshot->trend_process_running_count_delta = delta_i64;
        snapshot->trend_process_running_direction = sysmon_trend_direction_i64(delta_i64);

        delta_i64 = sysmon_trend_delta_u64(snapshot->process_sleeping_count, state->previous_snapshot.process_sleeping_count);
        snapshot->trend_process_sleeping_count_delta = delta_i64;
        snapshot->trend_process_sleeping_direction = sysmon_trend_direction_i64(delta_i64);
    }

    state->previous_snapshot = *snapshot;
    state->has_previous = 1;
    snapshot->sample_count++;
    return 0;
}

static void sysmon_trend_shutdown(sysmon_module_t *module) {
    sysmon_trend_state_t *state;

    if (module == NULL || module->userdata == NULL) {
        return;
    }

    state = (sysmon_trend_state_t *)module->userdata;
    state->has_previous = 0;
    memset(&state->previous_snapshot, 0, sizeof(state->previous_snapshot));
}

sysmon_module_t sysmon_trend_module_create(void) {
    static sysmon_trend_state_t state;
    sysmon_module_t module;

    module.name = "trend";
    module.interval_ms = SYSMON_DEFAULT_INTERVAL_MS;
    module.userdata = &state;
    module.init = sysmon_trend_init;
    module.collect = sysmon_trend_collect;
    module.shutdown = sysmon_trend_shutdown;

    return module;
}
