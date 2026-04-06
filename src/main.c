#include <stdio.h>

#include "sysmon/core/app.h"

int main(void) {
    sysmon_app_t app;

    if (sysmon_app_init(&app) != 0) {
        fprintf(stderr, "error: no se pudo inicializar app\n");
        return 1;
    }

    if (sysmon_app_register_defaults(&app) != 0) {
        fprintf(stderr, "error: no se pudieron registrar módulos por defecto\n");
        sysmon_app_shutdown(&app);
        return 1;
    }

    if (sysmon_app_run(&app) != 0) {
        fprintf(stderr, "error: fallo en ejecución\n");
        sysmon_app_shutdown(&app);
        return 1;
    }

    printf("[REPORT DATA: %s]\n", app.snapshot.identity_hostname[0] ? app.snapshot.identity_hostname : "unknown");
    printf("[STATUS: VERIFIED SYSTEM NODES...]\n");
    printf("user: %s\n", app.snapshot.identity_username[0] ? app.snapshot.identity_username : "unknown");
    printf("kernel: %s\n", app.snapshot.identity_kernel[0] ? app.snapshot.identity_kernel : "unknown");
    printf("uptime_s: %llu\n", (unsigned long long)app.snapshot.identity_uptime_seconds);
    printf("samples: %zu\n", app.snapshot.sample_count);

    sysmon_app_shutdown(&app);
    return 0;
}
