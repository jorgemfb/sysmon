#include "sysmon/core/app.h"
#include "sysmon/transport/message.h"
#include "sysmon/transport/server.h"

#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

int main(void) {
    sysmon_app_t app;
    sysmon_transport_server_t server;
    sysmon_transport_message_t outgoing;
    pid_t child_pid;
    int status = 0;
    const unsigned short port = (unsigned short)(32000u + ((unsigned)getpid() % 10000u));

    if (sysmon_transport_server_init(&server, "127.0.0.1", port) != 0 ||
        sysmon_transport_server_start(&server, 1) != 0 ||
        sysmon_transport_server_port(&server) != port) {
        fprintf(stderr, "test_client_mode: server setup failed\n");
        return 1;
    }

    if (sysmon_transport_message_init(&outgoing, SYSMON_TRANSPORT_MESSAGE_TYPE_REPORT) != 0 ||
        sysmon_transport_message_set_payload_string(&outgoing, "integration-client-message") != 0) {
        fprintf(stderr, "test_client_mode: outgoing message setup failed\n");
        (void)sysmon_transport_server_shutdown(&server);
        return 1;
    }

    child_pid = fork();
    if (child_pid < 0) {
        fprintf(stderr, "test_client_mode: fork failed\n");
        (void)sysmon_transport_server_shutdown(&server);
        return 1;
    }

    if (child_pid == 0) {
        if (sysmon_transport_server_accept_one(&server) != 0 ||
            sysmon_transport_server_send_message(&server, &outgoing) != 0) {
            _exit(10);
        }
        (void)sysmon_transport_server_shutdown(&server);
        _exit(0);
    }

    if (sysmon_app_init(&app) != 0) {
        fprintf(stderr, "test_client_mode: app init failed\n");
        (void)sysmon_transport_server_shutdown(&server);
        return 1;
    }
    app.context.config.mode = "client";

    if (sysmon_app_run_client(&app, "127.0.0.1", port) != 0 ||
        !app.snapshot.report_markdown_generated ||
        app.snapshot.report_markdown_length != strlen("integration-client-message") ||
        memcmp(app.snapshot.report_markdown, "integration-client-message", app.snapshot.report_markdown_length) != 0) {
        fprintf(stderr, "test_client_mode: app client mode receive failed\n");
        sysmon_app_shutdown(&app);
        (void)sysmon_transport_server_shutdown(&server);
        return 1;
    }

    sysmon_app_shutdown(&app);

    if (waitpid(child_pid, &status, 0) != child_pid || !WIFEXITED(status) || WEXITSTATUS(status) != 0) {
        fprintf(stderr, "test_client_mode: client validation failed\n");
        (void)sysmon_transport_server_shutdown(&server);
        return 1;
    }

    (void)sysmon_transport_server_shutdown(&server);
    printf("test_client_mode: ok\n");
    return 0;
}
