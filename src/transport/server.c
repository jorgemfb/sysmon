#include "sysmon/transport/server.h"

#include "sysmon/transport/serializer.h"
#include "sysmon/transport/socket.h"

#include <stddef.h>
#include <string.h>

#define SYSMON_TRANSPORT_SERVER_DEFAULT_BIND "127.0.0.1"
#define SYSMON_TRANSPORT_SERVER_ENCODED_MAX (SYSMON_TRANSPORT_MAX_PAYLOAD_SIZE + 16u)

static void sysmon_transport_server_copy_text(char *dst, size_t dst_size, const char *src) {
    size_t n;

    if (dst == NULL || dst_size == 0) {
        return;
    }

    if (src == NULL) {
        dst[0] = '\0';
        return;
    }

    n = strnlen(src, dst_size - 1);
    memcpy(dst, src, n);
    dst[n] = '\0';
}

int sysmon_transport_server_init(sysmon_transport_server_t *server, const char *bind_address, uint16_t port) {
    if (server == NULL) {
        return -1;
    }

    memset(server, 0, sizeof(*server));
    server->listen_fd = -1;
    server->client_fd = -1;
    server->port = port;
    sysmon_transport_server_copy_text(server->bind_address,
                                      sizeof(server->bind_address),
                                      (bind_address != NULL && bind_address[0] != '\0') ? bind_address : SYSMON_TRANSPORT_SERVER_DEFAULT_BIND);
    return 0;
}

int sysmon_transport_server_start(sysmon_transport_server_t *server, int backlog) {
    uint16_t bound_port;
    int listen_fd;

    if (server == NULL || backlog <= 0) {
        return -1;
    }

    listen_fd = sysmon_transport_socket_create_tcp();
    if (listen_fd < 0) {
        return -1;
    }

    if (sysmon_transport_socket_bind(listen_fd, server->bind_address, server->port) != 0 ||
        sysmon_transport_socket_listen(listen_fd, backlog) != 0 ||
        sysmon_transport_socket_get_bound_port(listen_fd, &bound_port) != 0) {
        (void)sysmon_transport_socket_close(listen_fd);
        return -1;
    }

    server->listen_fd = listen_fd;
    server->port = bound_port;
    return 0;
}

int sysmon_transport_server_accept_one(sysmon_transport_server_t *server) {
    int client_fd;

    if (server == NULL || server->listen_fd < 0) {
        return -1;
    }

    client_fd = sysmon_transport_socket_accept(server->listen_fd);
    if (client_fd < 0) {
        return -1;
    }

    if (server->client_fd >= 0) {
        (void)sysmon_transport_socket_close(server->client_fd);
    }
    server->client_fd = client_fd;
    return 0;
}

int sysmon_transport_server_send_message(sysmon_transport_server_t *server, const sysmon_transport_message_t *message) {
    unsigned char encoded[SYSMON_TRANSPORT_SERVER_ENCODED_MAX];
    size_t encoded_size = 0;

    if (server == NULL || message == NULL || server->client_fd < 0) {
        return -1;
    }

    if (sysmon_transport_serialize_message(message, encoded, sizeof(encoded), &encoded_size) != 0) {
        return -1;
    }

    return sysmon_transport_socket_send_all(server->client_fd, encoded, encoded_size);
}

int sysmon_transport_server_shutdown(sysmon_transport_server_t *server) {
    if (server == NULL) {
        return -1;
    }

    if (server->client_fd >= 0) {
        (void)sysmon_transport_socket_close(server->client_fd);
        server->client_fd = -1;
    }

    if (server->listen_fd >= 0) {
        (void)sysmon_transport_socket_close(server->listen_fd);
        server->listen_fd = -1;
    }

    return 0;
}

uint16_t sysmon_transport_server_port(const sysmon_transport_server_t *server) {
    if (server == NULL) {
        return 0;
    }
    return server->port;
}
