#include "sysmon/core/app.h"
#include "sysmon/modules/cpu/cpu_module.h"
#include "sysmon/modules/disk/disk_module.h"
#include "sysmon/modules/filesystem/filesystem_module.h"
#include "sysmon/modules/identity/identity_module.h"
#include "sysmon/modules/log/log_module.h"
#include "sysmon/modules/memory/memory_module.h"
#include "sysmon/modules/network/network_module.h"
#include "sysmon/modules/package/package_module.h"
#include "sysmon/modules/process/process_module.h"
#include "sysmon/modules/report/report_module.h"
#include "sysmon/modules/service/service_module.h"
#include "sysmon/modules/smart/smart_module.h"
#include "sysmon/modules/temperature/temperature_module.h"
#include "sysmon/modules/trend/trend_module.h"
#include "sysmon/transport/client.h"
#include "sysmon/transport/message.h"
#include "sysmon/transport/server.h"
#include "sysmon/util/time.h"

#include <stddef.h>
#include <string.h>

int sysmon_app_init(sysmon_app_t *app) {
    if (app == NULL) {
        return -1;
    }

    if (sysmon_context_init(&app->context) != 0) {
        return -1;
    }

    if (sysmon_registry_init(&app->registry) != 0) {
        return -1;
    }

    if (sysmon_scheduler_init(&app->scheduler) != 0) {
        return -1;
    }

    if (sysmon_snapshot_init(&app->snapshot) != 0) {
        return -1;
    }

    return 0;
}

int sysmon_app_register_defaults(sysmon_app_t *app) {
    sysmon_module_t module;
    int init_rc = 0;
    int add_rc = 0;

    if (app == NULL) {
        return -1;
    }

    module = sysmon_identity_module_create();
    if (module.init != NULL) {
        init_rc = module.init(&module, &app->context);
        if (init_rc != 0) {
            return -2;
        }
    }

    add_rc = sysmon_registry_add(&app->registry, &module);
    if (add_rc != 0) {
        return -3;
    }

    module = sysmon_memory_module_create();
    if (module.init != NULL) {
        init_rc = module.init(&module, &app->context);
        if (init_rc != 0) {
            return -4;
        }
    }

    add_rc = sysmon_registry_add(&app->registry, &module);
    if (add_rc != 0) {
        return -5;
    }

    module = sysmon_cpu_module_create();
    if (module.init != NULL) {
        init_rc = module.init(&module, &app->context);
        if (init_rc != 0) {
            return -6;
        }
    }

    add_rc = sysmon_registry_add(&app->registry, &module);
    if (add_rc != 0) {
        return -7;
    }

    module = sysmon_disk_module_create();
    if (module.init != NULL) {
        init_rc = module.init(&module, &app->context);
        if (init_rc != 0) {
            return -8;
        }
    }

    add_rc = sysmon_registry_add(&app->registry, &module);
    if (add_rc != 0) {
        return -9;
    }

    module = sysmon_filesystem_module_create();
    if (module.init != NULL) {
        init_rc = module.init(&module, &app->context);
        if (init_rc != 0) {
            return -10;
        }
    }

    add_rc = sysmon_registry_add(&app->registry, &module);
    if (add_rc != 0) {
        return -11;
    }

    module = sysmon_network_module_create();
    if (module.init != NULL) {
        init_rc = module.init(&module, &app->context);
        if (init_rc != 0) {
            return -12;
        }
    }

    add_rc = sysmon_registry_add(&app->registry, &module);
    if (add_rc != 0) {
        return -13;
    }

    module = sysmon_process_module_create();
    if (module.init != NULL) {
        init_rc = module.init(&module, &app->context);
        if (init_rc != 0) {
            return -14;
        }
    }

    add_rc = sysmon_registry_add(&app->registry, &module);
    if (add_rc != 0) {
        return -15;
    }

    module = sysmon_service_module_create();
    if (module.init != NULL) {
        init_rc = module.init(&module, &app->context);
        if (init_rc != 0) {
            return -16;
        }
    }

    add_rc = sysmon_registry_add(&app->registry, &module);
    if (add_rc != 0) {
        return -17;
    }

    module = sysmon_temperature_module_create();
    if (module.init != NULL) {
        init_rc = module.init(&module, &app->context);
        if (init_rc != 0) {
            return -18;
        }
    }

    add_rc = sysmon_registry_add(&app->registry, &module);
    if (add_rc != 0) {
        return -19;
    }

    module = sysmon_smart_module_create();
    if (module.init != NULL) {
        init_rc = module.init(&module, &app->context);
        if (init_rc != 0) {
            return -20;
        }
    }

    add_rc = sysmon_registry_add(&app->registry, &module);
    if (add_rc != 0) {
        return -21;
    }

    module = sysmon_log_module_create();
    if (module.init != NULL) {
        init_rc = module.init(&module, &app->context);
        if (init_rc != 0) {
            return -22;
        }
    }

    add_rc = sysmon_registry_add(&app->registry, &module);
    if (add_rc != 0) {
        return -23;
    }

    module = sysmon_package_module_create();
    if (module.init != NULL) {
        init_rc = module.init(&module, &app->context);
        if (init_rc != 0) {
            return -24;
        }
    }

    add_rc = sysmon_registry_add(&app->registry, &module);
    if (add_rc != 0) {
        return -25;
    }

    module = sysmon_trend_module_create();
    if (module.init != NULL) {
        init_rc = module.init(&module, &app->context);
        if (init_rc != 0) {
            return -26;
        }
    }

    add_rc = sysmon_registry_add(&app->registry, &module);
    if (add_rc != 0) {
        return -27;
    }

    module = sysmon_report_module_create();
    if (module.init != NULL) {
        init_rc = module.init(&module, &app->context);
        if (init_rc != 0) {
            return -28;
        }
    }

    add_rc = sysmon_registry_add(&app->registry, &module);
    if (add_rc != 0) {
        return -29;
    }

    return 0;
}

static int sysmon_app_run_local_runtime(sysmon_app_t *app) {
    int rc;

    if (app == NULL) {
        return -1;
    }

    rc = sysmon_scheduler_run_once(&app->scheduler, &app->registry, &app->snapshot);
    if (rc != 0) {
        return rc;
    }

    if (sysmon_time_sleep_ms(app->context.config.default_interval_ms) != 0) {
        return -2;
    }

    return sysmon_scheduler_run_once(&app->scheduler, &app->registry, &app->snapshot);
}

int sysmon_app_run_local(sysmon_app_t *app) {
    if (app != NULL) {
        app->context.config.mode = "local";
    }
    return sysmon_app_run_local_runtime(app);
}

int sysmon_app_run_server(sysmon_app_t *app, const char *bind_address, uint16_t port) {
    int rc = 0;
    int server_ready = 0;
    const char *payload_text;
    sysmon_transport_server_t server;
    sysmon_transport_message_t message;

    if (app == NULL) {
        return -1;
    }
    app->context.config.mode = "server";
    memset(&server, 0, sizeof(server));
    server.listen_fd = -1;
    server.client_fd = -1;

    if (sysmon_app_run_local_runtime(app) != 0) {
        return -2;
    }

    payload_text = app->snapshot.report_markdown_generated ? app->snapshot.report_markdown : "sysmon report unavailable";
    if (payload_text == NULL || payload_text[0] == '\0') {
        payload_text = "sysmon report unavailable";
    }

    if (sysmon_transport_server_init(&server, bind_address, port) != 0) {
        return -3;
    }
    server_ready = 1;
    if (sysmon_transport_server_start(&server, 1) != 0) {
        rc = -4;
        goto cleanup;
    }
    if (sysmon_transport_message_init(&message, SYSMON_TRANSPORT_MESSAGE_TYPE_REPORT) != 0 ||
        sysmon_transport_message_set_payload_string(&message, payload_text) != 0) {
        rc = -5;
        goto cleanup;
    }
    if (sysmon_transport_server_accept_one(&server) != 0) {
        rc = -6;
        goto cleanup;
    }
    if (sysmon_transport_server_send_message(&server, &message) != 0) {
        rc = -7;
        goto cleanup;
    }

cleanup:
    if (server_ready) {
        (void)sysmon_transport_server_shutdown(&server);
    }

    return rc;
}

int sysmon_app_run_client(sysmon_app_t *app, const char *server_address, uint16_t port) {
    int rc = 0;
    int client_ready = 0;
    size_t copy_len;
    sysmon_transport_client_t client;
    sysmon_transport_message_t incoming;

    if (app == NULL) {
        return -1;
    }
    app->context.config.mode = "client";
    app->snapshot.report_markdown[0] = '\0';
    app->snapshot.report_markdown_length = 0;
    app->snapshot.report_markdown_generated = 0;

    if (sysmon_transport_client_init(&client, server_address, port) != 0) {
        return -2;
    }
    client_ready = 1;
    if (sysmon_transport_client_connect(&client) != 0) {
        rc = -3;
        goto cleanup;
    }
    if (sysmon_transport_client_receive_message(&client, &incoming) != 0) {
        rc = -4;
        goto cleanup;
    }
    if (incoming.type != SYSMON_TRANSPORT_MESSAGE_TYPE_REPORT) {
        rc = -5;
        goto cleanup;
    }

    copy_len = incoming.payload_length;
    if (copy_len >= sizeof(app->snapshot.report_markdown)) {
        copy_len = sizeof(app->snapshot.report_markdown) - 1;
    }

    if (copy_len > 0) {
        memcpy(app->snapshot.report_markdown, incoming.payload, copy_len);
    }
    app->snapshot.report_markdown[copy_len] = '\0';
    app->snapshot.report_markdown_length = copy_len;
    app->snapshot.report_markdown_generated = 1;

cleanup:
    if (client_ready) {
        (void)sysmon_transport_client_shutdown(&client);
    }

    return rc;
}

int sysmon_app_run(sysmon_app_t *app) {
    return sysmon_app_run_local(app);
}

void sysmon_app_shutdown(sysmon_app_t *app) {
    size_t i;

    if (app == NULL) {
        return;
    }

    for (i = 0; i < app->registry.count; ++i) {
        sysmon_module_t *module = sysmon_registry_get(&app->registry, i);
        if (module != NULL && module->shutdown != NULL) {
            module->shutdown(module);
        }
    }

    app->registry.count = 0;
}
