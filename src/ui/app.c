#include "sysmon/ui/app.h"

#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <termios.h>
#include <unistd.h>

#define SYSMON_UI_PREVIEW_LIMIT 5u
#define SYSMON_UI_ALERT_LIMIT 10u
#define SYSMON_UI_ALERT_MSG_PREVIEW 96u
#define SYSMON_UI_REMOTE_EXCERPT_LINES 14u
#define SYSMON_UI_REMOTE_EXCERPT_CHARS 110u

enum {
    SYSMON_UI_KEY_NONE = 0,
    SYSMON_UI_KEY_UP = 1,
    SYSMON_UI_KEY_DOWN = 2,
    SYSMON_UI_KEY_QUIT = 3
};

typedef struct sysmon_ui_style {
    const char *reset;
    const char *title;
    const char *section;
    const char *divider;
    const char *key;
    const char *muted;
    const char *critical;
    const char *warning;
    const char *info;
    int color_enabled;
} sysmon_ui_style_t;

static const sysmon_ui_style_t *sysmon_ui_style_for_output(FILE *out) {
    static const sysmon_ui_style_t plain = {
        "", "", "", "", "", "", "", "", "", 0
    };
    static const sysmon_ui_style_t color = {
        "\033[0m",     /* reset */
        "\033[1;37m",  /* title */
        "\033[1;36m",  /* section */
        "\033[2;37m",  /* divider */
        "\033[1;34m",  /* key */
        "\033[2;37m",  /* muted */
        "\033[1;31m",  /* critical */
        "\033[1;33m",  /* warning */
        "\033[1;32m",  /* info */
        1
    };
    int out_fd;
    const char *term;

    if (out == NULL) {
        return &plain;
    }

    out_fd = fileno(out);
    term = getenv("TERM");
    if (out_fd >= 0 &&
        isatty(out_fd) &&
        term != NULL &&
        strcmp(term, "dumb") != 0 &&
        getenv("NO_COLOR") == NULL) {
        return &color;
    }

    return &plain;
}

static void sysmon_ui_print_divider(FILE *out) {
    const sysmon_ui_style_t *style = sysmon_ui_style_for_output(out);
    (void)fprintf(out, "%s------------------------------------------------------------------------%s\n",
                  style->divider, style->reset);
}

static void sysmon_ui_print_kv_str(FILE *out, const char *key, const char *value) {
    const sysmon_ui_style_t *style = sysmon_ui_style_for_output(out);
    (void)fprintf(out, "%s%-18s%s : %s\n",
                  style->key, key, style->reset, value != NULL ? value : "unknown");
}

static void sysmon_ui_print_kv_u64(FILE *out, const char *key, unsigned long long value) {
    const sysmon_ui_style_t *style = sysmon_ui_style_for_output(out);
    (void)fprintf(out, "%s%-18s%s : %llu\n", style->key, key, style->reset, value);
}

static void sysmon_ui_print_kv_f64(FILE *out, const char *key, double value) {
    const sysmon_ui_style_t *style = sysmon_ui_style_for_output(out);
    (void)fprintf(out, "%s%-18s%s : %.2f\n", style->key, key, style->reset, value);
}

static void sysmon_ui_print_section(FILE *out, const char *title) {
    const sysmon_ui_style_t *style = sysmon_ui_style_for_output(out);
    (void)fprintf(out, "\n%s## %s%s\n", style->section, title, style->reset);
    sysmon_ui_print_divider(out);
}

static const char *sysmon_ui_alert_label(const char *severity) {
    if (severity == NULL || severity[0] == '\0') {
        return "INFO";
    }
    if (strcasecmp(severity, "critical") == 0) {
        return "CRITICAL";
    }
    if (strcasecmp(severity, "warning") == 0) {
        return "WARNING";
    }
    return "INFO";
}

static const char *sysmon_ui_severity_color(const sysmon_ui_style_t *style, const char *severity_label) {
    if (style == NULL || severity_label == NULL) {
        return "";
    }
    if (strcmp(severity_label, "CRITICAL") == 0) {
        return style->critical;
    }
    if (strcmp(severity_label, "WARNING") == 0) {
        return style->warning;
    }
    return style->info;
}

static void sysmon_ui_format_severity(FILE *out,
                                      const char *severity,
                                      char *buffer,
                                      size_t buffer_size) {
    const sysmon_ui_style_t *style = sysmon_ui_style_for_output(out);
    const char *label;
    const char *color_code;

    if (buffer == NULL || buffer_size == 0) {
        return;
    }
    buffer[0] = '\0';

    label = sysmon_ui_alert_label(severity);
    color_code = sysmon_ui_severity_color(style, label);

    if (!style->color_enabled) {
        (void)snprintf(buffer, buffer_size, "%s", label);
        return;
    }

    (void)snprintf(buffer, buffer_size, "%s%s%s", color_code, label, style->reset);
}

static void sysmon_ui_extract_markdown_title(const char *markdown,
                                             size_t markdown_len,
                                             char *out,
                                             size_t out_size) {
    const char *begin;
    const char *end;
    size_t len;

    if (out == NULL || out_size == 0) {
        return;
    }
    out[0] = '\0';

    if (markdown == NULL || markdown_len == 0) {
        (void)snprintf(out, out_size, "not available");
        return;
    }

    begin = markdown;
    if (!(begin[0] == '#' && begin[1] == ' ')) {
        (void)snprintf(out, out_size, "unknown");
        return;
    }

    begin += 2;
    end = begin;
    while ((size_t)(end - markdown) < markdown_len && *end != '\n' && *end != '\r') {
        end++;
    }

    len = (size_t)(end - begin);
    if (len >= out_size) {
        len = out_size - 1;
    }

    memcpy(out, begin, len);
    out[len] = '\0';
}

static void sysmon_ui_extract_markdown_generated_at(const char *markdown,
                                                    size_t markdown_len,
                                                    char *out,
                                                    size_t out_size) {
    const char *tag = "- Generated: `";
    const char *start;
    const char *end;
    size_t len;

    if (out == NULL || out_size == 0) {
        return;
    }
    out[0] = '\0';

    if (markdown == NULL || markdown_len == 0) {
        (void)snprintf(out, out_size, "not available");
        return;
    }

    start = strstr(markdown, tag);
    if (start == NULL) {
        (void)snprintf(out, out_size, "unknown");
        return;
    }
    if ((size_t)(start - markdown) >= markdown_len) {
        (void)snprintf(out, out_size, "unknown");
        return;
    }

    start += strlen(tag);
    if ((size_t)(start - markdown) >= markdown_len) {
        (void)snprintf(out, out_size, "unknown");
        return;
    }

    end = strchr(start, '`');
    if (end == NULL || (size_t)(end - markdown) > markdown_len) {
        (void)snprintf(out, out_size, "unknown");
        return;
    }

    len = (size_t)(end - start);
    if (len >= out_size) {
        len = out_size - 1;
    }

    memcpy(out, start, len);
    out[len] = '\0';
}

static void sysmon_ui_extract_report_title(const sysmon_snapshot_t *snapshot, char *out, size_t out_size) {
    if (snapshot == NULL || !snapshot->report_markdown_generated || snapshot->report_markdown_length == 0) {
        if (out != NULL && out_size > 0) {
            (void)snprintf(out, out_size, "not available");
        }
        return;
    }

    sysmon_ui_extract_markdown_title(snapshot->report_markdown,
                                     snapshot->report_markdown_length,
                                     out,
                                     out_size);
}

static void sysmon_ui_extract_report_generated_at(const sysmon_snapshot_t *snapshot, char *out, size_t out_size) {
    if (snapshot == NULL || !snapshot->report_markdown_generated || snapshot->report_markdown_length == 0) {
        if (out != NULL && out_size > 0) {
            (void)snprintf(out, out_size, "not available");
        }
        return;
    }

    sysmon_ui_extract_markdown_generated_at(snapshot->report_markdown,
                                            snapshot->report_markdown_length,
                                            out,
                                            out_size);
}

static void sysmon_ui_count_alerts(const sysmon_snapshot_t *snapshot,
                                   size_t *critical_out,
                                   size_t *warning_out,
                                   size_t *info_out) {
    size_t i;
    size_t critical = 0;
    size_t warning = 0;
    size_t info = 0;

    if (snapshot != NULL) {
        for (i = 0; i < snapshot->alert_count; ++i) {
            const char *label = sysmon_ui_alert_label(snapshot->alert_entries[i].severity);
            if (strcmp(label, "CRITICAL") == 0) {
                critical++;
            } else if (strcmp(label, "WARNING") == 0) {
                warning++;
            } else {
                info++;
            }
        }
    }

    if (critical_out != NULL) {
        *critical_out = critical;
    }
    if (warning_out != NULL) {
        *warning_out = warning;
    }
    if (info_out != NULL) {
        *info_out = info;
    }
}

static void sysmon_ui_count_markdown_alerts(const char *payload,
                                            size_t payload_length,
                                            size_t *critical_out,
                                            size_t *warning_out,
                                            size_t *info_out) {
    size_t i = 0;
    size_t critical = 0;
    size_t warning = 0;
    size_t info = 0;

    if (payload != NULL) {
        while (i < payload_length) {
            size_t line_start = i;
            size_t line_end;
            size_t line_len;
            const char *line;
            size_t token_len = 0;

            while (i < payload_length && payload[i] != '\n') {
                i++;
            }
            line_end = i;
            if (i < payload_length && payload[i] == '\n') {
                i++;
            }

            line = payload + line_start;
            line_len = line_end - line_start;
            while (line_len > 0 && (*line == ' ' || *line == '\t' || *line == '\r')) {
                line++;
                line_len--;
            }

            if (line_len < 5 || line[0] != '-' || line[1] != ' ' || line[2] != '[') {
                continue;
            }

            while ((3u + token_len) < line_len && line[3u + token_len] != ']') {
                token_len++;
            }
            if ((3u + token_len) >= line_len || token_len == 0) {
                continue;
            }

            if (token_len == 8 && strncasecmp(line + 3, "critical", token_len) == 0) {
                critical++;
            } else if (token_len == 7 && strncasecmp(line + 3, "warning", token_len) == 0) {
                warning++;
            } else if (token_len == 4 && strncasecmp(line + 3, "info", token_len) == 0) {
                info++;
            }
        }
    }

    if (critical_out != NULL) {
        *critical_out = critical;
    }
    if (warning_out != NULL) {
        *warning_out = warning;
    }
    if (info_out != NULL) {
        *info_out = info;
    }
}

static void sysmon_ui_print_report_excerpt(FILE *out,
                                           const char *payload,
                                           size_t payload_length,
                                           size_t max_lines,
                                           size_t max_chars) {
    size_t i = 0;
    size_t printed = 0;

    if (out == NULL) {
        return;
    }

    if (payload == NULL || payload_length == 0) {
        (void)fprintf(out, "  (empty remote report payload)\n");
        return;
    }

    while (i < payload_length && printed < max_lines) {
        size_t line_start = i;
        size_t line_end;
        size_t line_len;
        int truncated;

        while (i < payload_length && payload[i] != '\n') {
            i++;
        }
        line_end = i;
        if (i < payload_length && payload[i] == '\n') {
            i++;
        }

        if (line_end > line_start && payload[line_end - 1] == '\r') {
            line_end--;
        }

        line_len = line_end - line_start;
        truncated = (line_len > max_chars);
        if (truncated) {
            line_len = max_chars;
        }

        (void)fprintf(out, "  %.*s%s\n",
                      (int)line_len,
                      payload + line_start,
                      truncated ? "..." : "");
        printed++;
    }

    if (i < payload_length) {
        (void)fprintf(out, "  ... (%zu more bytes)\n", payload_length - i);
    }
}

static int sysmon_ui_can_interact(FILE *out) {
    int out_fd;

    if (out == NULL) {
        return 0;
    }

    out_fd = fileno(out);
    if (out_fd < 0) {
        return 0;
    }

    return isatty(out_fd) && isatty(STDIN_FILENO);
}

static int sysmon_ui_enable_raw_mode(struct termios *original_out) {
    struct termios raw;

    if (original_out == NULL) {
        return -1;
    }

    if (tcgetattr(STDIN_FILENO, original_out) != 0) {
        return -1;
    }

    raw = *original_out;
    raw.c_iflag &= (tcflag_t)~(ICRNL | IXON);
    raw.c_lflag &= (tcflag_t)~(ECHO | ICANON);
    raw.c_cc[VMIN] = 1;
    raw.c_cc[VTIME] = 0;

    return tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

static void sysmon_ui_restore_raw_mode(const struct termios *original, int enabled) {
    if (enabled && original != NULL) {
        (void)tcsetattr(STDIN_FILENO, TCSAFLUSH, original);
    }
}

static int sysmon_ui_read_key(void) {
    int first = getchar();

    if (first == EOF) {
        return SYSMON_UI_KEY_QUIT;
    }

    if (first == 'q' || first == 'Q') {
        return SYSMON_UI_KEY_QUIT;
    }

    if (first == 'k' || first == 'K') {
        return SYSMON_UI_KEY_UP;
    }

    if (first == 'j' || first == 'J') {
        return SYSMON_UI_KEY_DOWN;
    }

    if (first == 27) {
        int second = getchar();
        if (second != '[') {
            return SYSMON_UI_KEY_NONE;
        }
        second = getchar();
        if (second == 'A') {
            return SYSMON_UI_KEY_UP;
        }
        if (second == 'B') {
            return SYSMON_UI_KEY_DOWN;
        }
    }

    return SYSMON_UI_KEY_NONE;
}

static int sysmon_ui_render_snapshot_once(FILE *out,
                                          const sysmon_snapshot_t *snapshot,
                                          const char *status,
                                          size_t selected_alert_index,
                                          int interactive) {
    const sysmon_ui_style_t *style;
    size_t i;
    size_t limit;
    size_t critical_count;
    size_t warning_count;
    size_t info_count;
    size_t alert_count;
    size_t alert_window_start = 0;
    size_t alert_window_end;
    const char *final_status;
    char report_title[96];
    char generated_at[96];

    if (out == NULL || snapshot == NULL) {
        return -1;
    }

    final_status = (status != NULL && status[0] != '\0') ? status : "local";
    style = sysmon_ui_style_for_output(out);
    alert_count = snapshot->alert_count;
    sysmon_ui_count_alerts(snapshot, &critical_count, &warning_count, &info_count);
    sysmon_ui_extract_report_title(snapshot, report_title, sizeof(report_title));
    sysmon_ui_extract_report_generated_at(snapshot, generated_at, sizeof(generated_at));
    if (alert_count > 0 && selected_alert_index >= alert_count) {
        selected_alert_index = alert_count - 1u;
    }

    (void)fprintf(out, "%s========================== SYSMON Terminal UI ==========================%s\n",
                  style->title, style->reset);
    sysmon_ui_print_kv_str(out, "status", final_status);
    (void)fprintf(out, "%s%-18s%s : %zu\n", style->key, "samples", style->reset, snapshot->sample_count);
    if (interactive) {
        (void)fprintf(out, "%s%-18s%s : %s%s%s\n",
                      style->key, "controls", style->reset,
                      style->muted, "j/down next | k/up prev | q quit", style->reset);
        if (alert_count > 0) {
            (void)fprintf(out, "%s%-18s%s : %zu/%zu\n",
                          style->key, "selected_alert", style->reset,
                          selected_alert_index + 1u, alert_count);
        }
    }
    sysmon_ui_print_divider(out);

    sysmon_ui_print_section(out, "Findings / Alerts Panel");
    sysmon_ui_print_kv_str(out, "evaluated", snapshot->alerts_evaluated ? "yes" : "no");
    (void)fprintf(out, "%-18s : %zu\n", "active_alerts", alert_count);
    if (alert_count == 0) {
        (void)fprintf(out, "  (no active alerts)\n");
    } else {
        if (alert_count > SYSMON_UI_ALERT_LIMIT && selected_alert_index >= SYSMON_UI_ALERT_LIMIT) {
            alert_window_start = selected_alert_index - SYSMON_UI_ALERT_LIMIT + 1u;
        }
        alert_window_end = alert_count;
        if ((alert_window_end - alert_window_start) > SYSMON_UI_ALERT_LIMIT) {
            alert_window_end = alert_window_start + SYSMON_UI_ALERT_LIMIT;
        }

        for (i = alert_window_start; i < alert_window_end; ++i) {
            char severity[64];
            const char *title = snapshot->alert_entries[i].title[0] ? snapshot->alert_entries[i].title : "untitled";
            const char *message = snapshot->alert_entries[i].message[0] ? snapshot->alert_entries[i].message : "(no message)";
            size_t msg_len = strnlen(message, SYSMON_UI_ALERT_MSG_PREVIEW + 1u);
            int truncated = msg_len > SYSMON_UI_ALERT_MSG_PREVIEW;
            int print_len = (int)(truncated ? SYSMON_UI_ALERT_MSG_PREVIEW : msg_len);
            sysmon_ui_format_severity(out, snapshot->alert_entries[i].severity, severity, sizeof(severity));
            (void)fprintf(out,
                          "  %s%c%s [%02zu] %-8s | %-24s | %.*s%s\n",
                          (i == selected_alert_index) ? style->section : "",
                          (i == selected_alert_index) ? '>' : ' ',
                          (i == selected_alert_index) ? style->reset : "",
                          i + 1u,
                          severity,
                          title,
                          print_len,
                          message,
                          truncated ? "..." : "");
        }

        if (alert_count > (alert_window_end - alert_window_start)) {
            (void)fprintf(out, "  showing %zu-%zu of %zu alerts\n",
                          alert_window_start + 1u, alert_window_end, alert_count);
        }
    }

    sysmon_ui_print_section(out, "Selected Alert Detail");
    if (alert_count == 0) {
        (void)fprintf(out, "  (no selected alert)\n");
    } else {
        char severity[64];
        const char *title = snapshot->alert_entries[selected_alert_index].title[0]
                                ? snapshot->alert_entries[selected_alert_index].title
                                : "untitled";
        const char *message = snapshot->alert_entries[selected_alert_index].message[0]
                                  ? snapshot->alert_entries[selected_alert_index].message
                                  : "(no message)";

        sysmon_ui_format_severity(out, snapshot->alert_entries[selected_alert_index].severity, severity, sizeof(severity));
        sysmon_ui_print_kv_str(out, "severity", severity);
        sysmon_ui_print_kv_str(out, "title", title);
        sysmon_ui_print_kv_str(out, "message", message);
    }

    sysmon_ui_print_section(out, "Report Summary");
    sysmon_ui_print_kv_str(out, "generated", snapshot->report_markdown_generated ? "yes" : "no");
    (void)fprintf(out, "%-18s : %zu\n", "markdown_bytes", snapshot->report_markdown_length);
    sysmon_ui_print_kv_str(out, "report_title", report_title);
    sysmon_ui_print_kv_str(out, "generated_at", generated_at);
    (void)fprintf(out, "%-18s : %zu\n", "alerts_critical", critical_count);
    (void)fprintf(out, "%-18s : %zu\n", "alerts_warning", warning_count);
    (void)fprintf(out, "%-18s : %zu\n", "alerts_info", info_count);
    (void)fprintf(out, "%-18s : snapshot -> findings -> report_markdown\n", "pipeline");

    sysmon_ui_print_section(out, "Identity Summary");
    sysmon_ui_print_kv_str(out, "hostname", snapshot->identity_hostname[0] ? snapshot->identity_hostname : "unknown");
    sysmon_ui_print_kv_str(out, "username", snapshot->identity_username[0] ? snapshot->identity_username : "unknown");
    sysmon_ui_print_kv_str(out, "kernel", snapshot->identity_kernel[0] ? snapshot->identity_kernel : "unknown");
    sysmon_ui_print_kv_u64(out, "uptime_s", (unsigned long long)snapshot->identity_uptime_seconds);

    sysmon_ui_print_section(out, "Memory Summary");
    sysmon_ui_print_kv_u64(out, "total_kib", (unsigned long long)snapshot->memory_total_kib);
    sysmon_ui_print_kv_u64(out, "available_kib", (unsigned long long)snapshot->memory_available_kib);
    sysmon_ui_print_kv_u64(out, "used_kib", (unsigned long long)snapshot->memory_used_kib);

    sysmon_ui_print_section(out, "CPU Summary");
    sysmon_ui_print_kv_f64(out, "usage_percent", snapshot->cpu_usage_percent);

    sysmon_ui_print_section(out, "Disk Summary");
    sysmon_ui_print_kv_str(out, "target", "/");
    sysmon_ui_print_kv_u64(out, "total_kib", (unsigned long long)snapshot->disk_total_kib);
    sysmon_ui_print_kv_u64(out, "available_kib", (unsigned long long)snapshot->disk_available_kib);
    sysmon_ui_print_kv_u64(out, "used_kib", (unsigned long long)snapshot->disk_used_kib);

    sysmon_ui_print_section(out, "Filesystem Summary");
    (void)fprintf(out, "%-18s : %zu\n", "count", snapshot->filesystem_count);
    if (snapshot->filesystem_count == 0) {
        (void)fprintf(out, "  (no filesystem data)\n");
    } else {
        limit = snapshot->filesystem_count < SYSMON_UI_PREVIEW_LIMIT ? snapshot->filesystem_count : SYSMON_UI_PREVIEW_LIMIT;
        for (i = 0; i < limit; ++i) {
            (void)fprintf(out, "  - %s (%s): total=%llu available=%llu used=%llu KiB\n",
                          snapshot->filesystem_entries[i].mountpoint,
                          snapshot->filesystem_entries[i].fs_type,
                          (unsigned long long)snapshot->filesystem_entries[i].total_kib,
                          (unsigned long long)snapshot->filesystem_entries[i].available_kib,
                          (unsigned long long)snapshot->filesystem_entries[i].used_kib);
        }
    }

    sysmon_ui_print_section(out, "Network Summary");
    (void)fprintf(out, "%-18s : %zu\n", "count", snapshot->network_count);
    if (snapshot->network_count == 0) {
        (void)fprintf(out, "  (no network data)\n");
    } else {
        limit = snapshot->network_count < SYSMON_UI_PREVIEW_LIMIT ? snapshot->network_count : SYSMON_UI_PREVIEW_LIMIT;
        for (i = 0; i < limit; ++i) {
            (void)fprintf(out, "  - %s: rx=%llu tx=%llu state=%s\n",
                          snapshot->network_entries[i].interface_name,
                          (unsigned long long)snapshot->network_entries[i].rx_bytes,
                          (unsigned long long)snapshot->network_entries[i].tx_bytes,
                          snapshot->network_entries[i].oper_state);
        }
    }

    sysmon_ui_print_section(out, "Process Summary");
    sysmon_ui_print_kv_u64(out, "total", (unsigned long long)snapshot->process_total_count);
    sysmon_ui_print_kv_u64(out, "running", (unsigned long long)snapshot->process_running_count);
    sysmon_ui_print_kv_u64(out, "sleeping", (unsigned long long)snapshot->process_sleeping_count);

    sysmon_ui_print_section(out, "Service Summary");
    sysmon_ui_print_kv_u64(out, "total", (unsigned long long)snapshot->service_total_count);
    sysmon_ui_print_kv_u64(out, "active", (unsigned long long)snapshot->service_active_count);
    sysmon_ui_print_kv_u64(out, "failed", (unsigned long long)snapshot->service_failed_count);

    sysmon_ui_print_section(out, "Temperature Summary");
    (void)fprintf(out, "%-18s : %zu\n", "count", snapshot->temperature_count);
    if (snapshot->temperature_count == 0) {
        (void)fprintf(out, "  (no temperature data)\n");
    } else {
        limit = snapshot->temperature_count < SYSMON_UI_PREVIEW_LIMIT ? snapshot->temperature_count : SYSMON_UI_PREVIEW_LIMIT;
        for (i = 0; i < limit; ++i) {
            (void)fprintf(out, "  - %s: %.2f C\n",
                          snapshot->temperature_entries[i].sensor_name,
                          snapshot->temperature_entries[i].temperature_c);
        }
    }

    sysmon_ui_print_section(out, "SMART Summary");
    (void)fprintf(out, "%-18s : %zu\n", "count", snapshot->smart_count);
    if (snapshot->smart_count == 0) {
        (void)fprintf(out, "  (no SMART data)\n");
    } else {
        limit = snapshot->smart_count < SYSMON_UI_PREVIEW_LIMIT ? snapshot->smart_count : SYSMON_UI_PREVIEW_LIMIT;
        for (i = 0; i < limit; ++i) {
            const char *support = snapshot->smart_entries[i].smart_supported == 1 ? "yes" :
                                  (snapshot->smart_entries[i].smart_supported == 0 ? "no" : "unknown");
            (void)fprintf(out, "  - %s: supported=%s health=%s\n",
                          snapshot->smart_entries[i].device_path,
                          support,
                          snapshot->smart_entries[i].smart_health);
        }
    }

    sysmon_ui_print_section(out, "Logs Summary");
    sysmon_ui_print_kv_str(out, "source_available", snapshot->log_source_available ? "yes" : "no");
    sysmon_ui_print_kv_u64(out, "considered", (unsigned long long)snapshot->log_total_entries_considered);
    sysmon_ui_print_kv_u64(out, "warnings", (unsigned long long)snapshot->log_warning_count);
    sysmon_ui_print_kv_u64(out, "errors", (unsigned long long)snapshot->log_error_count);
    if (snapshot->log_count == 0) {
        (void)fprintf(out, "  (no stored warnings/errors)\n");
    } else {
        limit = snapshot->log_count < SYSMON_UI_PREVIEW_LIMIT ? snapshot->log_count : SYSMON_UI_PREVIEW_LIMIT;
        for (i = 0; i < limit; ++i) {
            (void)fprintf(out, "  - [%s] %s\n", snapshot->log_entries[i].level, snapshot->log_entries[i].message);
        }
    }

    sysmon_ui_print_section(out, "Package Summary");
    sysmon_ui_print_kv_str(out, "manager", snapshot->package_manager[0] ? snapshot->package_manager : "unknown");
    sysmon_ui_print_kv_str(out, "source_available", snapshot->package_source_available ? "yes" : "no");
    sysmon_ui_print_kv_u64(out, "installed", (unsigned long long)snapshot->package_total_installed_count);
    sysmon_ui_print_kv_u64(out, "explicit", (unsigned long long)snapshot->package_explicit_installed_count);

    sysmon_ui_print_section(out, "Trend Summary");
    if (!snapshot->trend_has_previous) {
        (void)fprintf(out, "baseline           : only (no previous snapshot)\n");
    } else {
        sysmon_ui_print_kv_f64(out, "cpu_delta", snapshot->trend_cpu_usage_delta);
        (void)fprintf(out, "%-18s : %lld\n", "memory_used_delta", (long long)snapshot->trend_memory_used_kib_delta);
        (void)fprintf(out, "%-18s : %lld\n", "disk_avail_delta", (long long)snapshot->trend_disk_available_kib_delta);
        (void)fprintf(out, "%-18s : %lld\n", "process_total_delta", (long long)snapshot->trend_process_total_count_delta);
    }

    (void)fprintf(out, "\n");
    (void)fflush(out);
    return 0;
}

int sysmon_ui_render_snapshot(FILE *out, const sysmon_snapshot_t *snapshot, const char *status) {
    struct termios original;
    int raw_enabled = 0;
    size_t selected_alert = 0;
    int key;

    if (out == NULL || snapshot == NULL) {
        return -1;
    }

    if (!sysmon_ui_can_interact(out) || snapshot->alert_count == 0) {
        return sysmon_ui_render_snapshot_once(out, snapshot, status, 0, 0);
    }

    if (sysmon_ui_enable_raw_mode(&original) != 0) {
        return sysmon_ui_render_snapshot_once(out, snapshot, status, 0, 0);
    }
    raw_enabled = 1;

    (void)fprintf(out, "\033[?25l");
    for (;;) {
        (void)fprintf(out, "\033[H\033[J");
        if (sysmon_ui_render_snapshot_once(out, snapshot, status, selected_alert, 1) != 0) {
            sysmon_ui_restore_raw_mode(&original, raw_enabled);
            (void)fprintf(out, "\033[?25h");
            return -1;
        }

        key = sysmon_ui_read_key();
        if (key == SYSMON_UI_KEY_QUIT) {
            break;
        }
        if (key == SYSMON_UI_KEY_UP && selected_alert > 0) {
            selected_alert--;
        } else if (key == SYSMON_UI_KEY_DOWN && (selected_alert + 1u) < snapshot->alert_count) {
            selected_alert++;
        }
    }

    sysmon_ui_restore_raw_mode(&original, raw_enabled);
    (void)fprintf(out, "\033[?25h\n");
    (void)fflush(out);
    return 0;
}

int sysmon_ui_render_remote_report(FILE *out,
                                   const char *remote_host,
                                   uint16_t remote_port,
                                   const char *payload,
                                   size_t payload_length) {
    const sysmon_ui_style_t *style;
    size_t critical_count = 0;
    size_t warning_count = 0;
    size_t info_count = 0;
    char title[96];
    char generated_at[96];
    const char *host = (remote_host != NULL && remote_host[0] != '\0') ? remote_host : "unknown";

    if (out == NULL) {
        return -1;
    }
    if (payload == NULL && payload_length > 0) {
        return -1;
    }
    if (payload == NULL) {
        payload_length = 0;
    }

    style = sysmon_ui_style_for_output(out);
    sysmon_ui_extract_markdown_title(payload, payload_length, title, sizeof(title));
    sysmon_ui_extract_markdown_generated_at(payload, payload_length, generated_at, sizeof(generated_at));
    sysmon_ui_count_markdown_alerts(payload, payload_length, &critical_count, &warning_count, &info_count);

    (void)fprintf(out, "%s========================== SYSMON Remote Client ========================%s\n",
                  style->title, style->reset);
    sysmon_ui_print_kv_str(out, "mode", "client");
    (void)fprintf(out, "%s%-18s%s : %s:%u\n", style->key, "remote", style->reset, host, (unsigned)remote_port);
    (void)fprintf(out, "%s%-18s%s : %zu\n", style->key, "payload_bytes", style->reset, payload_length);
    sysmon_ui_print_divider(out);

    sysmon_ui_print_section(out, "Remote Report Summary");
    sysmon_ui_print_kv_str(out, "report_title", title);
    sysmon_ui_print_kv_str(out, "generated_at", generated_at);
    (void)fprintf(out, "%s%-18s%s : %s%zu%s\n", style->key, "alerts_critical", style->reset,
                  style->critical, critical_count, style->reset);
    (void)fprintf(out, "%s%-18s%s : %s%zu%s\n", style->key, "alerts_warning", style->reset,
                  style->warning, warning_count, style->reset);
    (void)fprintf(out, "%s%-18s%s : %s%zu%s\n", style->key, "alerts_info", style->reset,
                  style->info, info_count, style->reset);
    (void)fprintf(out, "%s%-18s%s : %zu\n", style->key, "alerts_total", style->reset,
                  critical_count + warning_count + info_count);
    (void)fprintf(out, "%s%-18s%s : %sserver snapshot -> report -> transport -> client%s\n",
                  style->key, "pipeline", style->reset, style->muted, style->reset);

    sysmon_ui_print_section(out, "Remote Report Excerpt");
    sysmon_ui_print_report_excerpt(out,
                                   payload,
                                   payload_length,
                                   SYSMON_UI_REMOTE_EXCERPT_LINES,
                                   SYSMON_UI_REMOTE_EXCERPT_CHARS);

    (void)fprintf(out, "\n");
    (void)fflush(out);
    return 0;
}
