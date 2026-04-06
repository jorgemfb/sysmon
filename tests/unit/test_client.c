#include "sysmon/transport/client.h"
#include "sysmon/transport/message.h"
#include "sysmon/transport/server.h"

#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

static int g_failures = 0;

static void expect_true(int condition, const char *message) {
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", message);
        g_failures++;
    }
}

int main(void) {
    sysmon_transport_server_t server;
    sysmon_transport_message_t outgoing;
    pid_t child_pid;
    int wait_status = 0;

    expect_true(sysmon_transport_server_init(&server, "127.0.0.1", 0) == 0, "server init must succeed");
    expect_true(sysmon_transport_server_start(&server, 1) == 0, "server start must succeed");
    expect_true(sysmon_transport_server_port(&server) > 0, "server port must be set");

    expect_true(sysmon_transport_message_init(&outgoing, SYSMON_TRANSPORT_MESSAGE_TYPE_ALERTS) == 0,
                "outgoing message init must succeed");
    expect_true(sysmon_transport_message_set_payload_string(&outgoing, "client-receive-payload") == 0,
                "outgoing message payload set must succeed");

    child_pid = fork();
    expect_true(child_pid >= 0, "fork must succeed");
    if (child_pid == 0) {
        sysmon_transport_client_t client;
        sysmon_transport_message_t incoming;

        if (sysmon_transport_client_init(&client, "127.0.0.1", sysmon_transport_server_port(&server)) != 0) {
            _exit(10);
        }
        if (sysmon_transport_client_connect(&client) != 0) {
            _exit(11);
        }
        if (sysmon_transport_client_receive_message(&client, &incoming) != 0) {
            (void)sysmon_transport_client_shutdown(&client);
            _exit(12);
        }
        if (incoming.type != SYSMON_TRANSPORT_MESSAGE_TYPE_ALERTS ||
            incoming.payload_length != strlen("client-receive-payload") ||
            memcmp(incoming.payload, "client-receive-payload", incoming.payload_length) != 0) {
            (void)sysmon_transport_client_shutdown(&client);
            _exit(13);
        }
        (void)sysmon_transport_client_shutdown(&client);
        _exit(0);
    }

    expect_true(sysmon_transport_server_accept_one(&server) == 0, "server accept must succeed");
    expect_true(sysmon_transport_server_send_message(&server, &outgoing) == 0, "server send must succeed");

    expect_true(waitpid(child_pid, &wait_status, 0) == child_pid, "waitpid must succeed");
    expect_true(WIFEXITED(wait_status), "child must exit normally");
    expect_true(WEXITSTATUS(wait_status) == 0, "child must validate received message");

    expect_true(sysmon_transport_server_shutdown(&server) == 0, "server shutdown must succeed");

    if (g_failures != 0) {
        return 1;
    }

    printf("test_client: ok\n");
    return 0;
}
