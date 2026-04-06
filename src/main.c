#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sysmon/core/app.h"
#include "sysmon/ui/app.h"

enum {
    SYSMON_EXIT_OK = 0,
    SYSMON_EXIT_USAGE = 2,
    SYSMON_EXIT_INIT = 3,
    SYSMON_EXIT_RUNTIME = 4,
    SYSMON_EXIT_RENDER = 5
};

typedef struct sysmon_main_options {
    const char *mode;
    const char *bind_address;
    const char *server_address;
    unsigned short port;
} sysmon_main_options_t;

static int sysmon_main_is_mode(const char *value) {
    if (value == NULL) {
        return 0;
    }
    return strcmp(value, "local") == 0 ||
           strcmp(value, "server") == 0 ||
           strcmp(value, "client") == 0;
}

static int sysmon_main_parse_port(const char *value, unsigned short *port_out) {
    unsigned long parsed;
    char *end = NULL;

    if (value == NULL || port_out == NULL) {
        return -1;
    }

    parsed = strtoul(value, &end, 10);
    if (end == value || *end != '\0' || parsed == 0ul || parsed > 65535ul) {
        return -1;
    }

    *port_out = (unsigned short)parsed;
    return 0;
}

static void sysmon_main_print_help(const char *argv0) {
    const char *prog = (argv0 != NULL && argv0[0] != '\0') ? argv0 : "sysmon";

    printf("sysmon v1\n");
    printf("\n");
    printf("Usage:\n");
    printf("  %s [--mode local|server|client] [--bind ADDR] [--host ADDR] [--port PORT]\n", prog);
    printf("  %s [local|server|client]\n", prog);
    printf("  %s --help\n", prog);
    printf("\n");
    printf("Modes:\n");
    printf("  local   Run local monitoring runtime and render local terminal UI (default).\n");
    printf("  server  Run local monitoring runtime and publish one report over transport.\n");
    printf("  client  Connect to a server and render the received remote report summary.\n");
    printf("\n");
    printf("Options:\n");
    printf("  --mode MODE   Execution mode: local, server, client.\n");
    printf("  --bind ADDR   Bind address for server mode (default: 127.0.0.1).\n");
    printf("  --host ADDR   Target server address for client mode (default: 127.0.0.1).\n");
    printf("  --port PORT   TCP port 1-65535 (default: 19090).\n");
    printf("  -h, --help    Show this help and exit.\n");
    printf("\n");
    printf("Examples:\n");
    printf("  %s\n", prog);
    printf("  %s --mode local\n", prog);
    printf("  %s --mode server --bind 127.0.0.1 --port 19090\n", prog);
    printf("  %s --mode client --host 127.0.0.1 --port 19090\n", prog);
}

static int sysmon_main_parse_args(int argc,
                                  char **argv,
                                  sysmon_main_options_t *options_out,
                                  int *show_help_out) {
    int i;
    int mode_already_set = 0;

    if (options_out == NULL || show_help_out == NULL) {
        return -1;
    }

    options_out->mode = "local";
    options_out->bind_address = "127.0.0.1";
    options_out->server_address = "127.0.0.1";
    options_out->port = 19090u;
    *show_help_out = 0;

    for (i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
            *show_help_out = 1;
            return 0;
        }

        if (strcmp(argv[i], "--mode") == 0) {
            if ((i + 1) >= argc) {
                fprintf(stderr, "error: missing value for --mode\n");
                return -1;
            }
            if (mode_already_set) {
                fprintf(stderr, "error: mode provided multiple times\n");
                return -1;
            }
            if (!sysmon_main_is_mode(argv[i + 1])) {
                fprintf(stderr, "error: invalid mode '%s'\n", argv[i + 1]);
                return -1;
            }
            options_out->mode = argv[++i];
            mode_already_set = 1;
            continue;
        }

        if (strcmp(argv[i], "--bind") == 0) {
            if ((i + 1) >= argc) {
                fprintf(stderr, "error: missing value for --bind\n");
                return -1;
            }
            options_out->bind_address = argv[++i];
            continue;
        }

        if (strcmp(argv[i], "--host") == 0) {
            if ((i + 1) >= argc) {
                fprintf(stderr, "error: missing value for --host\n");
                return -1;
            }
            options_out->server_address = argv[++i];
            continue;
        }

        if (strcmp(argv[i], "--port") == 0) {
            if ((i + 1) >= argc) {
                fprintf(stderr, "error: missing value for --port\n");
                return -1;
            }
            if (sysmon_main_parse_port(argv[++i], &options_out->port) != 0) {
                fprintf(stderr, "error: invalid port '%s' (expected 1-65535)\n", argv[i]);
                return -1;
            }
            continue;
        }

        if (argv[i][0] == '-') {
            fprintf(stderr, "error: unknown option '%s'\n", argv[i]);
            return -1;
        }

        if (!sysmon_main_is_mode(argv[i])) {
            fprintf(stderr, "error: unexpected argument '%s'\n", argv[i]);
            return -1;
        }

        if (mode_already_set) {
            fprintf(stderr, "error: mode provided multiple times\n");
            return -1;
        }
        options_out->mode = argv[i];
        mode_already_set = 1;
    }

    if (options_out->bind_address == NULL || options_out->bind_address[0] == '\0') {
        fprintf(stderr, "error: --bind cannot be empty\n");
        return -1;
    }
    if (options_out->server_address == NULL || options_out->server_address[0] == '\0') {
        fprintf(stderr, "error: --host cannot be empty\n");
        return -1;
    }

    return 0;
}

int main(int argc, char **argv) {
    sysmon_app_t app;
    sysmon_main_options_t options;
    const char *prog = (argc > 0 && argv != NULL && argv[0] != NULL && argv[0][0] != '\0')
                           ? argv[0]
                           : "sysmon";
    int show_help = 0;
    int app_initialized = 0;
    int run_rc;
    int exit_code = SYSMON_EXIT_OK;

    if (sysmon_main_parse_args(argc, argv, &options, &show_help) != 0) {
        fprintf(stderr, "Run '%s --help' for usage.\n", prog);
        return SYSMON_EXIT_USAGE;
    }

    if (show_help) {
        sysmon_main_print_help(prog);
        return SYSMON_EXIT_OK;
    }

    if (sysmon_app_init(&app) != 0) {
        fprintf(stderr, "error: failed to initialize application\n");
        return SYSMON_EXIT_INIT;
    }
    app_initialized = 1;

    if ((strcmp(options.mode, "local") == 0 || strcmp(options.mode, "server") == 0) &&
        sysmon_app_register_defaults(&app) != 0) {
        fprintf(stderr, "error: failed to register default modules\n");
        exit_code = SYSMON_EXIT_INIT;
        goto cleanup;
    }

    app.context.config.mode = options.mode;

    if (strcmp(options.mode, "local") == 0) {
        run_rc = sysmon_app_run_local(&app);
    } else if (strcmp(options.mode, "server") == 0) {
        run_rc = sysmon_app_run_server(&app, options.bind_address, options.port);
    } else if (strcmp(options.mode, "client") == 0) {
        run_rc = sysmon_app_run_client(&app, options.server_address, options.port);
    } else {
        fprintf(stderr, "error: unsupported mode '%s'\n", options.mode);
        exit_code = SYSMON_EXIT_USAGE;
        goto cleanup;
    }

    if (run_rc != 0) {
        fprintf(stderr, "error: execution failed in %s mode (rc=%d)\n", options.mode, run_rc);
        exit_code = SYSMON_EXIT_RUNTIME;
        goto cleanup;
    }

    if (strcmp(options.mode, "client") == 0) {
        if (sysmon_ui_render_remote_report(stdout,
                                           options.server_address,
                                           options.port,
                                           app.snapshot.report_markdown,
                                           app.snapshot.report_markdown_length) != 0) {
            fprintf(stderr, "error: failed to render remote client output\n");
            exit_code = SYSMON_EXIT_RENDER;
            goto cleanup;
        }
    } else if (strcmp(options.mode, "local") == 0) {
        if (sysmon_ui_render_snapshot(stdout, &app.snapshot, "local-mode active") != 0) {
            fprintf(stderr, "error: failed to render local UI\n");
            exit_code = SYSMON_EXIT_RENDER;
            goto cleanup;
        }
    } else {
        printf("[REPORT DATA: %s]\n", app.snapshot.identity_hostname[0] ? app.snapshot.identity_hostname : "unknown");
        printf("[STATUS: VERIFIED SYSTEM NODES...]\n");
        printf("user: %s\n", app.snapshot.identity_username[0] ? app.snapshot.identity_username : "unknown");
        printf("kernel: %s\n", app.snapshot.identity_kernel[0] ? app.snapshot.identity_kernel : "unknown");
        printf("uptime_s: %llu\n", (unsigned long long)app.snapshot.identity_uptime_seconds);
        printf("samples: %zu\n", app.snapshot.sample_count);
    }

cleanup:
    if (app_initialized) {
        sysmon_app_shutdown(&app);
    }

    return exit_code;
}
