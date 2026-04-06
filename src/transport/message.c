#include "sysmon/transport/message.h"

#include <string.h>

int sysmon_transport_message_init(sysmon_transport_message_t *message, uint32_t type) {
    if (message == NULL) {
        return -1;
    }

    if (!sysmon_transport_protocol_is_valid_type(type)) {
        return -1;
    }

    memset(message, 0, sizeof(*message));
    message->version = SYSMON_TRANSPORT_PROTOCOL_VERSION;
    message->type = type;
    return 0;
}

int sysmon_transport_message_set_payload(sysmon_transport_message_t *message, const void *payload, size_t payload_length) {
    if (message == NULL) {
        return -1;
    }

    if (payload_length > SYSMON_TRANSPORT_MAX_PAYLOAD_SIZE) {
        return -1;
    }

    if (payload_length > 0 && payload == NULL) {
        return -1;
    }

    if (payload_length > 0) {
        memcpy(message->payload, payload, payload_length);
    }
    message->payload_length = payload_length;
    return 0;
}

int sysmon_transport_message_set_payload_string(sysmon_transport_message_t *message, const char *payload_text) {
    size_t payload_length;

    if (message == NULL || payload_text == NULL) {
        return -1;
    }

    payload_length = strlen(payload_text);
    return sysmon_transport_message_set_payload(message, payload_text, payload_length);
}
