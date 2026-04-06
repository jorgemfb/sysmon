#include "sysmon/core/app.h"

#include <stdio.h>
#include <string.h>

int main(void) {
    sysmon_app_t app;

    if (sysmon_app_init(&app) != 0) {
        fprintf(stderr, "test_markdown_report: app init failed\n");
        return 1;
    }

    if (sysmon_app_register_defaults(&app) != 0) {
        fprintf(stderr, "test_markdown_report: app register defaults failed\n");
        sysmon_app_shutdown(&app);
        return 1;
    }

    if (sysmon_app_run(&app) != 0) {
        fprintf(stderr, "test_markdown_report: app run failed\n");
        sysmon_app_shutdown(&app);
        return 1;
    }

    if (!app.snapshot.report_markdown_generated ||
        !app.snapshot.alerts_evaluated ||
        app.snapshot.report_markdown_length == 0 ||
        strstr(app.snapshot.report_markdown, "# Sysmon Report") == NULL ||
        strstr(app.snapshot.report_markdown, "## Identity Summary") == NULL ||
        strstr(app.snapshot.report_markdown, "## Trend Summary") == NULL ||
        strstr(app.snapshot.report_markdown, "## Alerts Summary") == NULL) {
        fprintf(stderr, "test_markdown_report: generated markdown report is invalid\n");
        sysmon_app_shutdown(&app);
        return 1;
    }

    sysmon_app_shutdown(&app);
    printf("test_markdown_report: ok\n");
    return 0;
}
