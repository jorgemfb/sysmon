#include "sysmon/transport/socket.h"

#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

static int sysmon_transport_socket_make_addr(const char *address, uint16_t port, struct sockaddr_in *addr_out) {
    const char *effective_address;

    if (addr_out == NULL) {
        return -1;
    }

    memset(addr_out, 0, sizeof(*addr_out));
    addr_out->sin_family = AF_INET;
    addr_out->sin_port = htons(port);

    effective_address = (address != NULL && address[0] != '\0') ? address : "127.0.0.1";
    if (inet_pton(AF_INET, effective_address, &addr_out->sin_addr) != 1) {
        return -1;
    }

    return 0;
}

int sysmon_transport_socket_create_tcp(void) {
    int socket_fd;
    int enable = 1;

    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0) {
        return -1;
    }

    (void)setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable));
    return socket_fd;
}

int sysmon_transport_socket_connect(int socket_fd, const char *address, uint16_t port) {
    struct sockaddr_in addr;

    if (socket_fd < 0 || sysmon_transport_socket_make_addr(address, port, &addr) != 0) {
        return -1;
    }

    if (connect(socket_fd, (const struct sockaddr *)&addr, sizeof(addr)) != 0) {
        return -1;
    }

    return 0;
}

int sysmon_transport_socket_bind(int socket_fd, const char *address, uint16_t port) {
    struct sockaddr_in addr;

    if (socket_fd < 0 || sysmon_transport_socket_make_addr(address, port, &addr) != 0) {
        return -1;
    }

    if (bind(socket_fd, (const struct sockaddr *)&addr, sizeof(addr)) != 0) {
        return -1;
    }

    return 0;
}

int sysmon_transport_socket_listen(int socket_fd, int backlog) {
    if (socket_fd < 0 || backlog <= 0) {
        return -1;
    }

    if (listen(socket_fd, backlog) != 0) {
        return -1;
    }

    return 0;
}

int sysmon_transport_socket_accept(int socket_fd) {
    if (socket_fd < 0) {
        return -1;
    }

    return accept(socket_fd, NULL, NULL);
}

ssize_t sysmon_transport_socket_send(int socket_fd, const void *buffer, size_t length) {
    if (socket_fd < 0 || (buffer == NULL && length > 0)) {
        return -1;
    }

    return send(socket_fd, buffer, length, 0);
}

ssize_t sysmon_transport_socket_receive(int socket_fd, void *buffer, size_t length) {
    if (socket_fd < 0 || (buffer == NULL && length > 0)) {
        return -1;
    }

    return recv(socket_fd, buffer, length, 0);
}

int sysmon_transport_socket_send_all(int socket_fd, const void *buffer, size_t length) {
    size_t offset = 0;
    const unsigned char *bytes = (const unsigned char *)buffer;

    if (socket_fd < 0 || (buffer == NULL && length > 0)) {
        return -1;
    }

    while (offset < length) {
        ssize_t sent = send(socket_fd, bytes + offset, length - offset, 0);
        if (sent < 0) {
            if (errno == EINTR) {
                continue;
            }
            return -1;
        }
        if (sent == 0) {
            return -1;
        }
        offset += (size_t)sent;
    }

    return 0;
}

int sysmon_transport_socket_receive_all(int socket_fd, void *buffer, size_t length) {
    size_t offset = 0;
    unsigned char *bytes = (unsigned char *)buffer;

    if (socket_fd < 0 || (buffer == NULL && length > 0)) {
        return -1;
    }

    while (offset < length) {
        ssize_t received = recv(socket_fd, bytes + offset, length - offset, 0);
        if (received < 0) {
            if (errno == EINTR) {
                continue;
            }
            return -1;
        }
        if (received == 0) {
            return -1;
        }
        offset += (size_t)received;
    }

    return 0;
}

int sysmon_transport_socket_get_bound_port(int socket_fd, uint16_t *port_out) {
    struct sockaddr_in addr;
    socklen_t addr_len = (socklen_t)sizeof(addr);

    if (socket_fd < 0 || port_out == NULL) {
        return -1;
    }

    if (getsockname(socket_fd, (struct sockaddr *)&addr, &addr_len) != 0) {
        return -1;
    }

    if (addr.sin_family != AF_INET) {
        return -1;
    }

    *port_out = ntohs(addr.sin_port);
    return 0;
}

int sysmon_transport_socket_close(int socket_fd) {
    if (socket_fd < 0) {
        return -1;
    }

    return close(socket_fd);
}
