#ifndef INCLUDE_SYSMON_TRANSPORT_SOCKET_H
#define INCLUDE_SYSMON_TRANSPORT_SOCKET_H

#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>

int sysmon_transport_socket_create_tcp(void);
int sysmon_transport_socket_connect(int socket_fd, const char *address, uint16_t port);
int sysmon_transport_socket_bind(int socket_fd, const char *address, uint16_t port);
int sysmon_transport_socket_listen(int socket_fd, int backlog);
int sysmon_transport_socket_accept(int socket_fd);
ssize_t sysmon_transport_socket_send(int socket_fd, const void *buffer, size_t length);
ssize_t sysmon_transport_socket_receive(int socket_fd, void *buffer, size_t length);
int sysmon_transport_socket_send_all(int socket_fd, const void *buffer, size_t length);
int sysmon_transport_socket_receive_all(int socket_fd, void *buffer, size_t length);
int sysmon_transport_socket_get_bound_port(int socket_fd, uint16_t *port_out);
int sysmon_transport_socket_close(int socket_fd);

#endif
