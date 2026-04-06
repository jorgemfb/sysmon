#ifndef INCLUDE_SYSMON_TRANSPORT_MESSAGE_H
#define INCLUDE_SYSMON_TRANSPORT_MESSAGE_H

#include <stddef.h>
#include <stdint.h>

#include "sysmon/transport/protocol.h"

#define SYSMON_TRANSPORT_MAX_PAYLOAD_SIZE 32768u

typedef struct sysmon_transport_message {
    uint32_t version;
    uint32_t type;
    size_t payload_length;
    unsigned char payload[SYSMON_TRANSPORT_MAX_PAYLOAD_SIZE];
} sysmon_transport_message_t;

int sysmon_transport_message_init(sysmon_transport_message_t *message, uint32_t type);
int sysmon_transport_message_set_payload(sysmon_transport_message_t *message, const void *payload, size_t payload_length);
int sysmon_transport_message_set_payload_string(sysmon_transport_message_t *message, const char *payload_text);

#endif
