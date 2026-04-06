#include "sysmon/report/markdown.h"

#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

static int sysmon_report_appendf(char *buffer, size_t buffer_size, size_t *offset, const char *fmt, ...) {
    int written;
    va_list args;

    if (buffer == NULL || offset == NULL || fmt == NULL || *offset >= buffer_size) {
        return -1;
    }

    va_start(args, fmt);
    written = vsnprintf(buffer + *offset, buffer_size - *offset, fmt, args);
    va_end(args);

    if (written < 0) {
        return -1;
    }

    if ((size_t)written >= (buffer_size - *offset)) {
        buffer[buffer_size - 1] = '\0';
        *offset = buffer_size - 1;
        return -1;
    }

    *offset += (size_t)written;
    return 0;
}

static const char *sysmon_report_trend_label(int direction) {
    if (direction > 0) {
        return "increased";
    }
    if (direction < 0) {
        return "decreased";
    }
    return "unchanged";
}

int sysmon_report_markdown_build(const sysmon_snapshot_t *snapshot,
                                 const char *title,
                                 const char *timestamp,
                                 char *buffer,
                                 size_t buffer_size,
                                 size_t *written_out) {
    size_t i;
    size_t offset = 0;
    size_t limit;
    const char *final_title;
    const char *final_timestamp;

    if (snapshot == NULL || buffer == NULL || buffer_size == 0) {
        return -1;
    }

    buffer[0] = '\0';
    final_title = (title != NULL && title[0] != '\0') ? title : "Sysmon Report";
    final_timestamp = (timestamp != NULL && timestamp[0] != '\0') ? timestamp : "unknown";

    if (sysmon_report_appendf(buffer, buffer_size, &offset, "# %s\n\n", final_title) != 0 ||
        sysmon_report_appendf(buffer, buffer_size, &offset, "- Generated: `%s`\n\n", final_timestamp) != 0) {
        return -2;
    }

    if (sysmon_report_appendf(buffer, buffer_size, &offset, "## Identity Summary\n\n") != 0 ||
        sysmon_report_appendf(buffer, buffer_size, &offset, "- Hostname: `%s`\n",
                              snapshot->identity_hostname[0] != '\0' ? snapshot->identity_hostname : "unknown") != 0 ||
        sysmon_report_appendf(buffer, buffer_size, &offset, "- Username: `%s`\n",
                              snapshot->identity_username[0] != '\0' ? snapshot->identity_username : "unknown") != 0 ||
        sysmon_report_appendf(buffer, buffer_size, &offset, "- Kernel: `%s`\n",
                              snapshot->identity_kernel[0] != '\0' ? snapshot->identity_kernel : "unknown") != 0 ||
        sysmon_report_appendf(buffer, buffer_size, &offset, "- Uptime (s): `%llu`\n\n",
                              (unsigned long long)snapshot->identity_uptime_seconds) != 0) {
        return -2;
    }

    if (sysmon_report_appendf(buffer, buffer_size, &offset, "## Memory Summary\n\n") != 0 ||
        sysmon_report_appendf(buffer, buffer_size, &offset, "- Total KiB: `%llu`\n",
                              (unsigned long long)snapshot->memory_total_kib) != 0 ||
        sysmon_report_appendf(buffer, buffer_size, &offset, "- Available KiB: `%llu`\n",
                              (unsigned long long)snapshot->memory_available_kib) != 0 ||
        sysmon_report_appendf(buffer, buffer_size, &offset, "- Used KiB: `%llu`\n\n",
                              (unsigned long long)snapshot->memory_used_kib) != 0) {
        return -2;
    }

    if (sysmon_report_appendf(buffer, buffer_size, &offset, "## CPU Summary\n\n") != 0 ||
        sysmon_report_appendf(buffer, buffer_size, &offset, "- Usage %%: `%.2f`\n\n", snapshot->cpu_usage_percent) != 0) {
        return -2;
    }

    if (sysmon_report_appendf(buffer, buffer_size, &offset, "## Disk Summary\n\n") != 0 ||
        sysmon_report_appendf(buffer, buffer_size, &offset, "- Target: `/`\n") != 0 ||
        sysmon_report_appendf(buffer, buffer_size, &offset, "- Total KiB: `%llu`\n",
                              (unsigned long long)snapshot->disk_total_kib) != 0 ||
        sysmon_report_appendf(buffer, buffer_size, &offset, "- Available KiB: `%llu`\n",
                              (unsigned long long)snapshot->disk_available_kib) != 0 ||
        sysmon_report_appendf(buffer, buffer_size, &offset, "- Used KiB: `%llu`\n\n",
                              (unsigned long long)snapshot->disk_used_kib) != 0) {
        return -2;
    }

    if (sysmon_report_appendf(buffer, buffer_size, &offset, "## Filesystem Summary\n\n") != 0 ||
        sysmon_report_appendf(buffer, buffer_size, &offset, "- Filesystems: `%zu`\n", snapshot->filesystem_count) != 0) {
        return -2;
    }

    if (snapshot->filesystem_count == 0) {
        if (sysmon_report_appendf(buffer, buffer_size, &offset, "- No filesystem entries available.\n\n") != 0) {
            return -2;
        }
    } else {
        limit = snapshot->filesystem_count < 5 ? snapshot->filesystem_count : 5;
        for (i = 0; i < limit; ++i) {
            if (sysmon_report_appendf(buffer, buffer_size, &offset, "- `%s` (%s): total `%llu` KiB, available `%llu` KiB\n",
                                      snapshot->filesystem_entries[i].mountpoint,
                                      snapshot->filesystem_entries[i].fs_type,
                                      (unsigned long long)snapshot->filesystem_entries[i].total_kib,
                                      (unsigned long long)snapshot->filesystem_entries[i].available_kib) != 0) {
                return -2;
            }
        }
        if (snapshot->filesystem_count > limit &&
            sysmon_report_appendf(buffer, buffer_size, &offset, "- ... and `%zu` more\n",
                                  snapshot->filesystem_count - limit) != 0) {
            return -2;
        }
        if (sysmon_report_appendf(buffer, buffer_size, &offset, "\n") != 0) {
            return -2;
        }
    }

    if (sysmon_report_appendf(buffer, buffer_size, &offset, "## Network Summary\n\n") != 0 ||
        sysmon_report_appendf(buffer, buffer_size, &offset, "- Interfaces: `%zu`\n", snapshot->network_count) != 0) {
        return -2;
    }

    if (snapshot->network_count == 0) {
        if (sysmon_report_appendf(buffer, buffer_size, &offset, "- No network entries available.\n\n") != 0) {
            return -2;
        }
    } else {
        limit = snapshot->network_count < 5 ? snapshot->network_count : 5;
        for (i = 0; i < limit; ++i) {
            if (sysmon_report_appendf(buffer, buffer_size, &offset, "- `%s`: rx `%llu`, tx `%llu`, state `%s`\n",
                                      snapshot->network_entries[i].interface_name,
                                      (unsigned long long)snapshot->network_entries[i].rx_bytes,
                                      (unsigned long long)snapshot->network_entries[i].tx_bytes,
                                      snapshot->network_entries[i].oper_state) != 0) {
                return -2;
            }
        }
        if (snapshot->network_count > limit &&
            sysmon_report_appendf(buffer, buffer_size, &offset, "- ... and `%zu` more\n",
                                  snapshot->network_count - limit) != 0) {
            return -2;
        }
        if (sysmon_report_appendf(buffer, buffer_size, &offset, "\n") != 0) {
            return -2;
        }
    }

    if (sysmon_report_appendf(buffer, buffer_size, &offset, "## Process Summary\n\n") != 0 ||
        sysmon_report_appendf(buffer, buffer_size, &offset, "- Total: `%llu`\n",
                              (unsigned long long)snapshot->process_total_count) != 0 ||
        sysmon_report_appendf(buffer, buffer_size, &offset, "- Running: `%llu`\n",
                              (unsigned long long)snapshot->process_running_count) != 0 ||
        sysmon_report_appendf(buffer, buffer_size, &offset, "- Sleeping: `%llu`\n\n",
                              (unsigned long long)snapshot->process_sleeping_count) != 0) {
        return -2;
    }

    if (sysmon_report_appendf(buffer, buffer_size, &offset, "## Service Summary\n\n") != 0 ||
        sysmon_report_appendf(buffer, buffer_size, &offset, "- Total: `%llu`\n",
                              (unsigned long long)snapshot->service_total_count) != 0 ||
        sysmon_report_appendf(buffer, buffer_size, &offset, "- Active: `%llu`\n",
                              (unsigned long long)snapshot->service_active_count) != 0 ||
        sysmon_report_appendf(buffer, buffer_size, &offset, "- Failed: `%llu`\n\n",
                              (unsigned long long)snapshot->service_failed_count) != 0) {
        return -2;
    }

    if (sysmon_report_appendf(buffer, buffer_size, &offset, "## Temperature Summary\n\n") != 0 ||
        sysmon_report_appendf(buffer, buffer_size, &offset, "- Sensors: `%zu`\n", snapshot->temperature_count) != 0) {
        return -2;
    }

    if (snapshot->temperature_count == 0) {
        if (sysmon_report_appendf(buffer, buffer_size, &offset, "- No temperature entries available.\n\n") != 0) {
            return -2;
        }
    } else {
        limit = snapshot->temperature_count < 5 ? snapshot->temperature_count : 5;
        for (i = 0; i < limit; ++i) {
            if (sysmon_report_appendf(buffer, buffer_size, &offset, "- `%s`: `%.2f` C\n",
                                      snapshot->temperature_entries[i].sensor_name,
                                      snapshot->temperature_entries[i].temperature_c) != 0) {
                return -2;
            }
        }
        if (snapshot->temperature_count > limit &&
            sysmon_report_appendf(buffer, buffer_size, &offset, "- ... and `%zu` more\n",
                                  snapshot->temperature_count - limit) != 0) {
            return -2;
        }
        if (sysmon_report_appendf(buffer, buffer_size, &offset, "\n") != 0) {
            return -2;
        }
    }

    if (sysmon_report_appendf(buffer, buffer_size, &offset, "## SMART Summary\n\n") != 0 ||
        sysmon_report_appendf(buffer, buffer_size, &offset, "- Devices: `%zu`\n", snapshot->smart_count) != 0) {
        return -2;
    }

    if (snapshot->smart_count == 0) {
        if (sysmon_report_appendf(buffer, buffer_size, &offset, "- No SMART entries available.\n\n") != 0) {
            return -2;
        }
    } else {
        limit = snapshot->smart_count < 5 ? snapshot->smart_count : 5;
        for (i = 0; i < limit; ++i) {
            if (sysmon_report_appendf(buffer, buffer_size, &offset, "- `%s`: support `%d`, health `%s`\n",
                                      snapshot->smart_entries[i].device_path,
                                      snapshot->smart_entries[i].smart_supported,
                                      snapshot->smart_entries[i].smart_health) != 0) {
                return -2;
            }
        }
        if (snapshot->smart_count > limit &&
            sysmon_report_appendf(buffer, buffer_size, &offset, "- ... and `%zu` more\n",
                                  snapshot->smart_count - limit) != 0) {
            return -2;
        }
        if (sysmon_report_appendf(buffer, buffer_size, &offset, "\n") != 0) {
            return -2;
        }
    }

    if (sysmon_report_appendf(buffer, buffer_size, &offset, "## Logs Summary\n\n") != 0 ||
        sysmon_report_appendf(buffer, buffer_size, &offset, "- Source available: `%s`\n",
                              snapshot->log_source_available ? "yes" : "no") != 0 ||
        sysmon_report_appendf(buffer, buffer_size, &offset, "- Considered entries: `%llu`\n",
                              (unsigned long long)snapshot->log_total_entries_considered) != 0 ||
        sysmon_report_appendf(buffer, buffer_size, &offset, "- Warnings: `%llu`\n",
                              (unsigned long long)snapshot->log_warning_count) != 0 ||
        sysmon_report_appendf(buffer, buffer_size, &offset, "- Errors: `%llu`\n",
                              (unsigned long long)snapshot->log_error_count) != 0 ||
        sysmon_report_appendf(buffer, buffer_size, &offset, "- Stored messages: `%zu`\n",
                              snapshot->log_count) != 0) {
        return -2;
    }

    if (snapshot->log_count > 0) {
        limit = snapshot->log_count < 5 ? snapshot->log_count : 5;
        for (i = 0; i < limit; ++i) {
            if (sysmon_report_appendf(buffer, buffer_size, &offset, "- [%s] %s\n",
                                      snapshot->log_entries[i].level,
                                      snapshot->log_entries[i].message) != 0) {
                return -2;
            }
        }
    } else if (sysmon_report_appendf(buffer, buffer_size, &offset, "- No warning/error messages stored.\n") != 0) {
        return -2;
    }

    if (sysmon_report_appendf(buffer, buffer_size, &offset, "\n") != 0) {
        return -2;
    }

    if (sysmon_report_appendf(buffer, buffer_size, &offset, "## Package Summary\n\n") != 0 ||
        sysmon_report_appendf(buffer, buffer_size, &offset, "- Package manager: `%s`\n",
                              snapshot->package_manager[0] != '\0' ? snapshot->package_manager : "unknown") != 0 ||
        sysmon_report_appendf(buffer, buffer_size, &offset, "- Source available: `%s`\n",
                              snapshot->package_source_available ? "yes" : "no") != 0 ||
        sysmon_report_appendf(buffer, buffer_size, &offset, "- Installed packages: `%llu`\n",
                              (unsigned long long)snapshot->package_total_installed_count) != 0 ||
        sysmon_report_appendf(buffer, buffer_size, &offset, "- Explicit packages: `%llu`\n\n",
                              (unsigned long long)snapshot->package_explicit_installed_count) != 0) {
        return -2;
    }

    if (sysmon_report_appendf(buffer, buffer_size, &offset, "## Trend Summary\n\n") != 0) {
        return -2;
    }

    if (!snapshot->trend_has_previous) {
        if (sysmon_report_appendf(buffer, buffer_size, &offset, "- Baseline only. No previous snapshot to compare.\n") != 0) {
            return -2;
        }
    } else {
        if (sysmon_report_appendf(buffer, buffer_size, &offset, "- CPU usage delta: `%.2f` (%s)\n",
                                  snapshot->trend_cpu_usage_delta,
                                  sysmon_report_trend_label(snapshot->trend_cpu_usage_direction)) != 0 ||
            sysmon_report_appendf(buffer, buffer_size, &offset, "- Memory used delta KiB: `%lld` (%s)\n",
                                  (long long)snapshot->trend_memory_used_kib_delta,
                                  sysmon_report_trend_label(snapshot->trend_memory_used_direction)) != 0 ||
            sysmon_report_appendf(buffer, buffer_size, &offset, "- Disk available delta KiB: `%lld` (%s)\n",
                                  (long long)snapshot->trend_disk_available_kib_delta,
                                  sysmon_report_trend_label(snapshot->trend_disk_available_direction)) != 0 ||
            sysmon_report_appendf(buffer, buffer_size, &offset, "- Process total delta: `%lld` (%s)\n",
                                  (long long)snapshot->trend_process_total_count_delta,
                                  sysmon_report_trend_label(snapshot->trend_process_total_direction)) != 0 ||
            sysmon_report_appendf(buffer, buffer_size, &offset, "- Process running delta: `%lld` (%s)\n",
                                  (long long)snapshot->trend_process_running_count_delta,
                                  sysmon_report_trend_label(snapshot->trend_process_running_direction)) != 0 ||
            sysmon_report_appendf(buffer, buffer_size, &offset, "- Process sleeping delta: `%lld` (%s)\n",
                                  (long long)snapshot->trend_process_sleeping_count_delta,
                                  sysmon_report_trend_label(snapshot->trend_process_sleeping_direction)) != 0) {
            return -2;
        }
    }

    if (sysmon_report_appendf(buffer, buffer_size, &offset, "\n## Alerts Summary\n\n") != 0 ||
        sysmon_report_appendf(buffer, buffer_size, &offset, "- Evaluated: `%s`\n",
                              snapshot->alerts_evaluated ? "yes" : "no") != 0 ||
        sysmon_report_appendf(buffer, buffer_size, &offset, "- Total alerts: `%zu`\n",
                              snapshot->alert_count) != 0) {
        return -2;
    }

    if (snapshot->alert_count == 0) {
        if (sysmon_report_appendf(buffer, buffer_size, &offset, "- No active alerts.\n") != 0) {
            return -2;
        }
    } else {
        limit = snapshot->alert_count < 10 ? snapshot->alert_count : 10;
        for (i = 0; i < limit; ++i) {
            if (sysmon_report_appendf(buffer,
                                      buffer_size,
                                      &offset,
                                      "- [%s] %s: %s\n",
                                      snapshot->alert_entries[i].severity,
                                      snapshot->alert_entries[i].title,
                                      snapshot->alert_entries[i].message) != 0) {
                return -2;
            }
        }
        if (snapshot->alert_count > limit &&
            sysmon_report_appendf(buffer, buffer_size, &offset, "- ... and `%zu` more\n", snapshot->alert_count - limit) != 0) {
            return -2;
        }
    }

    if (written_out != NULL) {
        *written_out = offset;
    }

    return 0;
}
