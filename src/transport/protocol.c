#include "sysmon/transport/protocol.h"

int sysmon_transport_protocol_is_supported_version(uint32_t version) {
    return version == SYSMON_TRANSPORT_PROTOCOL_VERSION;
}

int sysmon_transport_protocol_is_valid_type(uint32_t type) {
    switch (type) {
        case SYSMON_TRANSPORT_MESSAGE_TYPE_SNAPSHOT:
        case SYSMON_TRANSPORT_MESSAGE_TYPE_REPORT:
        case SYSMON_TRANSPORT_MESSAGE_TYPE_ALERTS:
        case SYSMON_TRANSPORT_MESSAGE_TYPE_HEARTBEAT:
            return 1;
        default:
            return 0;
    }
}

const char *sysmon_transport_protocol_type_to_string(uint32_t type) {
    switch (type) {
        case SYSMON_TRANSPORT_MESSAGE_TYPE_SNAPSHOT:
            return "snapshot";
        case SYSMON_TRANSPORT_MESSAGE_TYPE_REPORT:
            return "report";
        case SYSMON_TRANSPORT_MESSAGE_TYPE_ALERTS:
            return "alerts";
        case SYSMON_TRANSPORT_MESSAGE_TYPE_HEARTBEAT:
            return "heartbeat";
        default:
            return "unknown";
    }
}
