#include "sysmon/transport/client.h"

#include "sysmon/transport/serializer.h"
#include "sysmon/transport/socket.h"

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#define SYSMON_TRANSPORT_CLIENT_DEFAULT_SERVER "127.0.0.1"
#define SYSMON_TRANSPORT_CLIENT_HEADER_SIZE 16u
#define SYSMON_TRANSPORT_CLIENT_FRAME_MAX (SYSMON_TRANSPORT_MAX_PAYLOAD_SIZE + SYSMON_TRANSPORT_CLIENT_HEADER_SIZE)

static void sysmon_transport_client_copy_text(char *dst, size_t dst_size, const char *src) {
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

static uint32_t sysmon_transport_client_read_u32_be(const unsigned char *src) {
    return ((uint32_t)src[0] << 24) |
           ((uint32_t)src[1] << 16) |
           ((uint32_t)src[2] << 8) |
           (uint32_t)src[3];
}

int sysmon_transport_client_init(sysmon_transport_client_t *client, const char *server_address, uint16_t port) {
    if (client == NULL) {
        return -1;
    }

    memset(client, 0, sizeof(*client));
    client->socket_fd = -1;
    client->port = port;
    sysmon_transport_client_copy_text(client->server_address,
                                      sizeof(client->server_address),
                                      (server_address != NULL && server_address[0] != '\0')
                                          ? server_address
                                          : SYSMON_TRANSPORT_CLIENT_DEFAULT_SERVER);
    return 0;
}

int sysmon_transport_client_connect(sysmon_transport_client_t *client) {
    int socket_fd;

    if (client == NULL) {
        return -1;
    }

    socket_fd = sysmon_transport_socket_create_tcp();
    if (socket_fd < 0) {
        return -1;
    }

    if (sysmon_transport_socket_connect(socket_fd, client->server_address, client->port) != 0) {
        (void)sysmon_transport_socket_close(socket_fd);
        return -1;
    }

    if (client->socket_fd >= 0) {
        (void)sysmon_transport_socket_close(client->socket_fd);
    }
    client->socket_fd = socket_fd;
    return 0;
}

int sysmon_transport_client_receive_message(sysmon_transport_client_t *client, sysmon_transport_message_t *message_out) {
    unsigned char frame[SYSMON_TRANSPORT_CLIENT_FRAME_MAX];
    uint32_t payload_len_u32;
    size_t payload_len;
    size_t frame_len;

    if (client == NULL || message_out == NULL || client->socket_fd < 0) {
        return -1;
    }

    if (sysmon_transport_socket_receive_all(client->socket_fd, frame, SYSMON_TRANSPORT_CLIENT_HEADER_SIZE) != 0) {
        return -1;
    }

    payload_len_u32 = sysmon_transport_client_read_u32_be(frame + 12);
    payload_len = (size_t)payload_len_u32;
    if (payload_len > SYSMON_TRANSPORT_MAX_PAYLOAD_SIZE) {
        return -1;
    }

    frame_len = SYSMON_TRANSPORT_CLIENT_HEADER_SIZE + payload_len;
    if (payload_len > 0 &&
        sysmon_transport_socket_receive_all(client->socket_fd, frame + SYSMON_TRANSPORT_CLIENT_HEADER_SIZE, payload_len) != 0) {
        return -1;
    }

    return sysmon_transport_deserialize_message(frame, frame_len, message_out);
}

int sysmon_transport_client_shutdown(sysmon_transport_client_t *client) {
    if (client == NULL) {
        return -1;
    }

    if (client->socket_fd >= 0) {
        (void)sysmon_transport_socket_close(client->socket_fd);
        client->socket_fd = -1;
    }
    return 0;
}
