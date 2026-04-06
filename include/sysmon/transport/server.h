#ifndef INCLUDE_SYSMON_TRANSPORT_SERVER_H
#define INCLUDE_SYSMON_TRANSPORT_SERVER_H

#include <stdint.h>

#include "sysmon/transport/message.h"

typedef struct sysmon_transport_server {
    int listen_fd;
    int client_fd;
    uint16_t port;
    char bind_address[64];
} sysmon_transport_server_t;

int sysmon_transport_server_init(sysmon_transport_server_t *server, const char *bind_address, uint16_t port);
int sysmon_transport_server_start(sysmon_transport_server_t *server, int backlog);
int sysmon_transport_server_accept_one(sysmon_transport_server_t *server);
int sysmon_transport_server_send_message(sysmon_transport_server_t *server, const sysmon_transport_message_t *message);
int sysmon_transport_server_shutdown(sysmon_transport_server_t *server);
uint16_t sysmon_transport_server_port(const sysmon_transport_server_t *server);

#endif
