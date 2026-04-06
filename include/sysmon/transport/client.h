#ifndef INCLUDE_SYSMON_TRANSPORT_CLIENT_H
#define INCLUDE_SYSMON_TRANSPORT_CLIENT_H

#include <stdint.h>

#include "sysmon/transport/message.h"

typedef struct sysmon_transport_client {
    int socket_fd;
    uint16_t port;
    char server_address[64];
} sysmon_transport_client_t;

int sysmon_transport_client_init(sysmon_transport_client_t *client, const char *server_address, uint16_t port);
int sysmon_transport_client_connect(sysmon_transport_client_t *client);
int sysmon_transport_client_receive_message(sysmon_transport_client_t *client, sysmon_transport_message_t *message_out);
int sysmon_transport_client_shutdown(sysmon_transport_client_t *client);

#endif
