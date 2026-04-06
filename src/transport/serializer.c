#include "sysmon/transport/serializer.h"

#include "sysmon/transport/protocol.h"

#include <stdint.h>
#include <string.h>

static const unsigned char SYSMON_TRANSPORT_MAGIC[4] = {'S', 'M', 'O', 'N'};
static const size_t SYSMON_TRANSPORT_HEADER_SIZE = 16u;

static void sysmon_transport_write_u32_be(unsigned char *dst, uint32_t value) {
    dst[0] = (unsigned char)((value >> 24) & 0xFFu);
    dst[1] = (unsigned char)((value >> 16) & 0xFFu);
    dst[2] = (unsigned char)((value >> 8) & 0xFFu);
    dst[3] = (unsigned char)(value & 0xFFu);
}

static uint32_t sysmon_transport_read_u32_be(const unsigned char *src) {
    return ((uint32_t)src[0] << 24) |
           ((uint32_t)src[1] << 16) |
           ((uint32_t)src[2] << 8) |
           (uint32_t)src[3];
}

int sysmon_transport_serialize_message(const sysmon_transport_message_t *message,
                                       unsigned char *buffer,
                                       size_t buffer_size,
                                       size_t *written_out) {
    size_t total_size;

    if (written_out != NULL) {
        *written_out = 0;
    }

    if (message == NULL || buffer == NULL) {
        return -1;
    }

    if (!sysmon_transport_protocol_is_supported_version(message->version) ||
        !sysmon_transport_protocol_is_valid_type(message->type)) {
        return -1;
    }

    if (message->payload_length > SYSMON_TRANSPORT_MAX_PAYLOAD_SIZE) {
        return -1;
    }

    total_size = SYSMON_TRANSPORT_HEADER_SIZE + message->payload_length;
    if (buffer_size < total_size) {
        return -2;
    }

    memcpy(buffer, SYSMON_TRANSPORT_MAGIC, sizeof(SYSMON_TRANSPORT_MAGIC));
    sysmon_transport_write_u32_be(buffer + 4, message->version);
    sysmon_transport_write_u32_be(buffer + 8, message->type);
    sysmon_transport_write_u32_be(buffer + 12, (uint32_t)message->payload_length);
    if (message->payload_length > 0) {
        memcpy(buffer + SYSMON_TRANSPORT_HEADER_SIZE, message->payload, message->payload_length);
    }

    if (written_out != NULL) {
        *written_out = total_size;
    }

    return 0;
}

int sysmon_transport_deserialize_message(const unsigned char *buffer,
                                         size_t buffer_size,
                                         sysmon_transport_message_t *message_out) {
    uint32_t version;
    uint32_t type;
    uint32_t payload_length_u32;
    size_t payload_length;

    if (buffer == NULL || message_out == NULL) {
        return -1;
    }

    if (buffer_size < SYSMON_TRANSPORT_HEADER_SIZE) {
        return -2;
    }

    if (memcmp(buffer, SYSMON_TRANSPORT_MAGIC, sizeof(SYSMON_TRANSPORT_MAGIC)) != 0) {
        return -2;
    }

    version = sysmon_transport_read_u32_be(buffer + 4);
    type = sysmon_transport_read_u32_be(buffer + 8);
    payload_length_u32 = sysmon_transport_read_u32_be(buffer + 12);
    payload_length = (size_t)payload_length_u32;

    if (!sysmon_transport_protocol_is_supported_version(version) ||
        !sysmon_transport_protocol_is_valid_type(type)) {
        return -2;
    }

    if (payload_length > SYSMON_TRANSPORT_MAX_PAYLOAD_SIZE) {
        return -2;
    }

    if (buffer_size != (SYSMON_TRANSPORT_HEADER_SIZE + payload_length)) {
        return -2;
    }

    memset(message_out, 0, sizeof(*message_out));
    message_out->version = version;
    message_out->type = type;
    message_out->payload_length = payload_length;

    if (payload_length > 0) {
        memcpy(message_out->payload, buffer + SYSMON_TRANSPORT_HEADER_SIZE, payload_length);
    }

    return 0;
}
