#include "sysmon/transport/message.h"
#include "sysmon/transport/serializer.h"
#include "sysmon/transport/server.h"
#include "sysmon/transport/socket.h"

#include <stdint.h>
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
    unsigned char expected_encoded[512];
    size_t expected_size = 0;
    pid_t child_pid;
    int wait_status = 0;
    int rc;

    rc = sysmon_transport_server_init(&server, "127.0.0.1", 0);
    expect_true(rc == 0, "server init must succeed");

    rc = sysmon_transport_server_start(&server, 1);
    expect_true(rc == 0, "server start must succeed");
    expect_true(sysmon_transport_server_port(&server) > 0, "server must expose bound port");

    expect_true(sysmon_transport_message_init(&outgoing, SYSMON_TRANSPORT_MESSAGE_TYPE_REPORT) == 0,
                "outgoing message init must succeed");
    expect_true(sysmon_transport_message_set_payload_string(&outgoing, "server-message") == 0,
                "outgoing message payload set must succeed");
    expect_true(sysmon_transport_serialize_message(&outgoing, expected_encoded, sizeof(expected_encoded), &expected_size) == 0,
                "serialize expected message must succeed");

    child_pid = fork();
    expect_true(child_pid >= 0, "fork must succeed");
    if (child_pid == 0) {
        int client_fd;
        unsigned char received[512];
        sysmon_transport_message_t decoded;
        int child_rc = 0;

        client_fd = sysmon_transport_socket_create_tcp();
        if (client_fd < 0) {
            _exit(10);
        }

        if (sysmon_transport_socket_connect(client_fd, "127.0.0.1", sysmon_transport_server_port(&server)) != 0) {
            (void)sysmon_transport_socket_close(client_fd);
            _exit(11);
        }

        if (sysmon_transport_socket_receive_all(client_fd, received, expected_size) != 0) {
            (void)sysmon_transport_socket_close(client_fd);
            _exit(12);
        }

        child_rc = sysmon_transport_deserialize_message(received, expected_size, &decoded);
        if (child_rc != 0 ||
            decoded.type != SYSMON_TRANSPORT_MESSAGE_TYPE_REPORT ||
            decoded.payload_length != strlen("server-message") ||
            memcmp(decoded.payload, "server-message", decoded.payload_length) != 0) {
            (void)sysmon_transport_socket_close(client_fd);
            _exit(13);
        }

        (void)sysmon_transport_socket_close(client_fd);
        _exit(0);
    }

    rc = sysmon_transport_server_accept_one(&server);
    expect_true(rc == 0, "server accept must succeed");

    rc = sysmon_transport_server_send_message(&server, &outgoing);
    expect_true(rc == 0, "server send message must succeed");

    expect_true(waitpid(child_pid, &wait_status, 0) == child_pid, "waitpid must succeed");
    expect_true(WIFEXITED(wait_status), "child must exit normally");
    expect_true(WEXITSTATUS(wait_status) == 0, "child must validate received server message");

    expect_true(sysmon_transport_server_shutdown(&server) == 0, "server shutdown must succeed");
    expect_true(sysmon_transport_server_send_message(&server, &outgoing) == -1,
                "server send must fail after shutdown");

    if (g_failures != 0) {
        return 1;
    }

    printf("test_server: ok\n");
    return 0;
}
