#ifndef INCLUDE_SYSMON_TRANSPORT_PROTOCOL_H
#define INCLUDE_SYSMON_TRANSPORT_PROTOCOL_H

#include <stdint.h>

#define SYSMON_TRANSPORT_PROTOCOL_VERSION 1u

typedef enum sysmon_transport_message_type {
    SYSMON_TRANSPORT_MESSAGE_TYPE_UNKNOWN = 0,
    SYSMON_TRANSPORT_MESSAGE_TYPE_SNAPSHOT = 1,
    SYSMON_TRANSPORT_MESSAGE_TYPE_REPORT = 2,
    SYSMON_TRANSPORT_MESSAGE_TYPE_ALERTS = 3,
    SYSMON_TRANSPORT_MESSAGE_TYPE_HEARTBEAT = 4
} sysmon_transport_message_type_t;

int sysmon_transport_protocol_is_supported_version(uint32_t version);
int sysmon_transport_protocol_is_valid_type(uint32_t type);
const char *sysmon_transport_protocol_type_to_string(uint32_t type);

#endif
