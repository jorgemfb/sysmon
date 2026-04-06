#include "sysmon/core/snapshot.h"
#include "sysmon/ui/app.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int g_failures = 0;

static void expect_true(int condition, const char *message) {
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", message);
        g_failures++;
    }
}

static char *read_all(FILE *fp) {
    long size;
    char *buffer;

    if (fp == NULL) {
        return NULL;
    }

    if (fseek(fp, 0, SEEK_END) != 0) {
        return NULL;
    }
    size = ftell(fp);
    if (size < 0) {
        return NULL;
    }
    if (fseek(fp, 0, SEEK_SET) != 0) {
        return NULL;
    }

    buffer = (char *)malloc((size_t)size + 1u);
    if (buffer == NULL) {
        return NULL;
    }
    if (size > 0 && fread(buffer, 1u, (size_t)size, fp) != (size_t)size) {
        free(buffer);
        return NULL;
    }
    buffer[size] = '\0';
    return buffer;
}

int main(void) {
    sysmon_snapshot_t snapshot;
    FILE *tmp;
    FILE *remote_tmp;
    char *rendered;
    char *remote_rendered;
    const char *remote_payload =
        "# Sysmon Report\n"
        "\n"
        "- Generated: `2026-04-07 10:00:00 CEST`\n"
        "\n"
        "## Alerts Summary\n"
        "\n"
        "- [critical] Disk Space Low: / has less than 5% free\n"
        "- [warning] High Memory Usage: memory pressure warning\n"
        "- [info] Baseline Updated: trend baseline refreshed\n";

    expect_true(sysmon_snapshot_init(&snapshot) == 0, "snapshot init must succeed");
    snapshot.identity_hostname[0] = 'h';
    snapshot.identity_hostname[1] = '\0';
    snapshot.memory_total_kib = 1;
    snapshot.cpu_usage_percent = 1.0;
    snapshot.disk_total_kib = 1;
    snapshot.alerts_evaluated = 1;
    snapshot.alert_count = 1;
    strcpy(snapshot.alert_entries[0].severity, "warning");
    strcpy(snapshot.alert_entries[0].title, "High Memory Usage");
    strcpy(snapshot.alert_entries[0].message, "Memory usage exceeded the warning threshold.");
    strcpy(snapshot.report_markdown, "# Sysmon Report\n\n- Generated: `2026-04-07 00:00:00 CEST`\n");
    snapshot.report_markdown_length = strlen(snapshot.report_markdown);
    snapshot.report_markdown_generated = 1;

    tmp = tmpfile();
    expect_true(tmp != NULL, "tmpfile must succeed");
    if (tmp == NULL) {
        return 1;
    }

    expect_true(sysmon_ui_render_snapshot(tmp, &snapshot, "test-status") == 0,
                "ui render must succeed");

    rendered = read_all(tmp);
    fclose(tmp);
    expect_true(rendered != NULL, "rendered output must be readable");
    if (rendered == NULL) {
        return 1;
    }

    expect_true(strstr(rendered, "SYSMON Terminal UI") != NULL, "title must exist");
    expect_true(strstr(rendered, "Findings / Alerts Panel") != NULL, "findings panel must exist");
    expect_true(strstr(rendered, "Selected Alert Detail") != NULL, "alert detail section must exist");
    expect_true(strstr(rendered, "Report Summary") != NULL, "report summary section must exist");
    expect_true(strstr(rendered, "WARNING") != NULL, "alert severity label must be visible");
    expect_true(strstr(rendered, "High Memory Usage") != NULL, "alert title must be visible");
    expect_true(strstr(rendered, "warning threshold") != NULL, "alert message must be visible");
    expect_true(strstr(rendered, "severity") != NULL, "detail severity field must exist");
    expect_true(strstr(rendered, "message") != NULL, "detail message field must exist");
    expect_true(strstr(rendered, "generated_at") != NULL, "report generation context field must exist");
    expect_true(strstr(rendered, "2026-04-07 00:00:00 CEST") != NULL, "report generation timestamp must be visible");
    expect_true(strstr(rendered, "alerts_warning") != NULL, "report alert warning count must be visible");
    expect_true(strstr(rendered, "pipeline") != NULL, "snapshot-findings-report relation must be visible");
    expect_true(strstr(rendered, "Identity Summary") != NULL, "identity section must exist");
    expect_true(strstr(rendered, "Memory Summary") != NULL, "memory section must exist");
    expect_true(strstr(rendered, "CPU Summary") != NULL, "cpu section must exist");
    expect_true(strstr(rendered, "Disk Summary") != NULL, "disk section must exist");
    expect_true(strstr(rendered, "Filesystem Summary") != NULL, "filesystem section must exist");
    expect_true(strstr(rendered, "Network Summary") != NULL, "network section must exist");
    expect_true(strstr(rendered, "Process Summary") != NULL, "process section must exist");
    expect_true(strstr(rendered, "Service Summary") != NULL, "service section must exist");
    expect_true(strstr(rendered, "Temperature Summary") != NULL, "temperature section must exist");
    expect_true(strstr(rendered, "SMART Summary") != NULL, "smart section must exist");
    expect_true(strstr(rendered, "Logs Summary") != NULL, "logs section must exist");
    expect_true(strstr(rendered, "Package Summary") != NULL, "package section must exist");
    expect_true(strstr(rendered, "Trend Summary") != NULL, "trend section must exist");

    free(rendered);

    remote_tmp = tmpfile();
    expect_true(remote_tmp != NULL, "remote tmpfile must succeed");
    if (remote_tmp == NULL) {
        return 1;
    }

    expect_true(sysmon_ui_render_remote_report(remote_tmp,
                                               "127.0.0.1",
                                               19090u,
                                               remote_payload,
                                               strlen(remote_payload)) == 0,
                "remote ui render must succeed");

    remote_rendered = read_all(remote_tmp);
    fclose(remote_tmp);
    expect_true(remote_rendered != NULL, "remote rendered output must be readable");
    if (remote_rendered == NULL) {
        return 1;
    }

    expect_true(strstr(remote_rendered, "SYSMON Remote Client") != NULL, "remote title must exist");
    expect_true(strstr(remote_rendered, "remote") != NULL, "remote context key must exist");
    expect_true(strstr(remote_rendered, "127.0.0.1:19090") != NULL, "remote endpoint must exist");
    expect_true(strstr(remote_rendered, "Remote Report Summary") != NULL, "remote report summary must exist");
    expect_true(strstr(remote_rendered, "2026-04-07 10:00:00 CEST") != NULL, "remote generated timestamp must exist");
    expect_true(strstr(remote_rendered, "alerts_critical") != NULL, "remote critical count key must exist");
    expect_true(strstr(remote_rendered, "alerts_warning") != NULL, "remote warning count key must exist");
    expect_true(strstr(remote_rendered, "alerts_info") != NULL, "remote info count key must exist");
    expect_true(strstr(remote_rendered, "Remote Report Excerpt") != NULL, "remote excerpt section must exist");
    expect_true(strstr(remote_rendered, "Disk Space Low") != NULL, "remote excerpt content must exist");

    free(remote_rendered);

    if (g_failures != 0) {
        return 1;
    }

    printf("test_ui: ok\n");
    return 0;
}
