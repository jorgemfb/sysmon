#include "sysmon/transport/message.h"
#include "sysmon/transport/serializer.h"
#include "sysmon/transport/socket.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

static int g_failures = 0;

static void expect_true(int condition, const char *message) {
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", message);
        g_failures++;
    }
}

int main(void) {
    int server_fd = -1;
    int client_fd = -1;
    int peer_fd = -1;
    uint16_t port = 0;
    unsigned char tx_buffer[512];
    unsigned char rx_buffer[512];
    size_t encoded_size = 0;
    sysmon_transport_message_t outgoing;
    sysmon_transport_message_t incoming;
    int rc;

    server_fd = sysmon_transport_socket_create_tcp();
    expect_true(server_fd >= 0, "server socket create must succeed");
    if (server_fd < 0) {
        return 1;
    }

    expect_true(sysmon_transport_socket_bind(server_fd, "127.0.0.1", 0) == 0,
                "server bind to loopback must succeed");
    expect_true(sysmon_transport_socket_get_bound_port(server_fd, &port) == 0,
                "server bound port lookup must succeed");
    expect_true(port > 0, "ephemeral bound port must be non-zero");
    expect_true(sysmon_transport_socket_listen(server_fd, 1) == 0,
                "server listen must succeed");

    client_fd = sysmon_transport_socket_create_tcp();
    expect_true(client_fd >= 0, "client socket create must succeed");
    if (client_fd < 0) {
        (void)sysmon_transport_socket_close(server_fd);
        return 1;
    }

    expect_true(sysmon_transport_socket_connect(client_fd, "127.0.0.1", port) == 0,
                "client connect to loopback server must succeed");
    peer_fd = sysmon_transport_socket_accept(server_fd);
    expect_true(peer_fd >= 0, "server accept must succeed");

    expect_true(sysmon_transport_message_init(&outgoing, SYSMON_TRANSPORT_MESSAGE_TYPE_SNAPSHOT) == 0,
                "transport message init must succeed");
    expect_true(sysmon_transport_message_set_payload_string(&outgoing, "socket-loopback-payload") == 0,
                "transport payload set must succeed");

    rc = sysmon_transport_serialize_message(&outgoing, tx_buffer, sizeof(tx_buffer), &encoded_size);
    expect_true(rc == 0, "transport message serialize must succeed");
    expect_true(encoded_size > 16, "serialized message must include payload bytes");

    rc = sysmon_transport_socket_send_all(client_fd, tx_buffer, encoded_size);
    expect_true(rc == 0, "client send_all must succeed");
    rc = sysmon_transport_socket_receive_all(peer_fd, rx_buffer, encoded_size);
    expect_true(rc == 0, "server receive_all must succeed");

    rc = sysmon_transport_deserialize_message(rx_buffer, encoded_size, &incoming);
    expect_true(rc == 0, "server deserialize must succeed");
    expect_true(incoming.type == SYSMON_TRANSPORT_MESSAGE_TYPE_SNAPSHOT,
                "server decoded type must match");
    expect_true(incoming.payload_length == strlen("socket-loopback-payload"),
                "server decoded payload length must match");
    expect_true(memcmp(incoming.payload, "socket-loopback-payload", incoming.payload_length) == 0,
                "server decoded payload bytes must match");

    rc = sysmon_transport_socket_send_all(peer_fd, tx_buffer, encoded_size);
    expect_true(rc == 0, "server send_all must succeed");
    rc = sysmon_transport_socket_receive_all(client_fd, rx_buffer, encoded_size);
    expect_true(rc == 0, "client receive_all must succeed");

    rc = sysmon_transport_deserialize_message(rx_buffer, encoded_size, &incoming);
    expect_true(rc == 0, "client deserialize must succeed");
    expect_true(incoming.type == SYSMON_TRANSPORT_MESSAGE_TYPE_SNAPSHOT,
                "client decoded type must match");

    expect_true(sysmon_transport_socket_connect(client_fd, "not-an-ip", port) == -1,
                "connect with invalid IP must fail");

    if (peer_fd >= 0) {
        (void)sysmon_transport_socket_close(peer_fd);
    }
    if (client_fd >= 0) {
        (void)sysmon_transport_socket_close(client_fd);
    }
    if (server_fd >= 0) {
        (void)sysmon_transport_socket_close(server_fd);
    }

    if (g_failures != 0) {
        return 1;
    }

    printf("test_socket: ok\n");
    return 0;
}
