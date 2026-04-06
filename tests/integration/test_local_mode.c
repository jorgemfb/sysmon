#include "sysmon/core/app.h"

#include <stdio.h>

int main(void) {
    sysmon_app_t app;

    if (sysmon_app_init(&app) != 0) {
        fprintf(stderr, "test_local_mode: app init failed\n");
        return 1;
    }

    app.context.config.mode = "local";
    app.context.config.default_interval_ms = 10;

    if (sysmon_app_register_defaults(&app) != 0) {
        fprintf(stderr, "test_local_mode: register defaults failed\n");
        sysmon_app_shutdown(&app);
        return 1;
    }

    if (sysmon_app_run_local(&app) != 0) {
        fprintf(stderr, "test_local_mode: local run failed\n");
        sysmon_app_shutdown(&app);
        return 1;
    }

    if (app.snapshot.sample_count == 0 ||
        !app.snapshot.report_markdown_generated ||
        app.snapshot.report_markdown_length == 0) {
        fprintf(stderr, "test_local_mode: invalid local snapshot/report data\n");
        sysmon_app_shutdown(&app);
        return 1;
    }

    sysmon_app_shutdown(&app);
    printf("test_local_mode: ok\n");
    return 0;
}
