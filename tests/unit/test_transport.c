#include "sysmon/transport/message.h"
#include "sysmon/transport/protocol.h"
#include "sysmon/transport/serializer.h"

#include <stdio.h>
#include <string.h>

static int g_failures = 0;

static void expect_true(int condition, const char *message) {
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", message);
        g_failures++;
    }
}

int main(void) {
    unsigned char buffer[512];
    unsigned char malformed[512];
    sysmon_transport_message_t message;
    sysmon_transport_message_t decoded;
    size_t encoded_size = 0;
    int rc;

    expect_true(sysmon_transport_protocol_is_supported_version(SYSMON_TRANSPORT_PROTOCOL_VERSION) == 1,
                "current transport protocol version must be supported");
    expect_true(sysmon_transport_protocol_is_supported_version(SYSMON_TRANSPORT_PROTOCOL_VERSION + 1u) == 0,
                "future transport protocol version must not be supported yet");
    expect_true(sysmon_transport_protocol_is_valid_type(SYSMON_TRANSPORT_MESSAGE_TYPE_REPORT) == 1,
                "report message type must be valid");
    expect_true(sysmon_transport_protocol_is_valid_type(SYSMON_TRANSPORT_MESSAGE_TYPE_UNKNOWN) == 0,
                "unknown message type must be invalid");

    expect_true(sysmon_transport_message_init(NULL, SYSMON_TRANSPORT_MESSAGE_TYPE_REPORT) == -1,
                "message init with NULL pointer must fail");
    expect_true(sysmon_transport_message_init(&message, SYSMON_TRANSPORT_MESSAGE_TYPE_UNKNOWN) == -1,
                "message init with invalid type must fail");

    expect_true(sysmon_transport_message_init(&message, SYSMON_TRANSPORT_MESSAGE_TYPE_REPORT) == 0,
                "message init must succeed");
    expect_true(message.version == SYSMON_TRANSPORT_PROTOCOL_VERSION,
                "message init must set protocol version");
    expect_true(message.type == SYSMON_TRANSPORT_MESSAGE_TYPE_REPORT,
                "message init must set message type");
    expect_true(message.payload_length == 0, "message init must set empty payload");

    rc = sysmon_transport_message_set_payload_string(&message, "report:ok");
    expect_true(rc == 0, "message string payload set must succeed");
    expect_true(message.payload_length == strlen("report:ok"),
                "message payload length must match string length");

    rc = sysmon_transport_serialize_message(&message, buffer, sizeof(buffer), &encoded_size);
    expect_true(rc == 0, "message serialization must succeed");
    expect_true(encoded_size > 16, "encoded message must include header and payload");

    rc = sysmon_transport_deserialize_message(buffer, encoded_size, &decoded);
    expect_true(rc == 0, "message deserialization must succeed");
    expect_true(decoded.version == SYSMON_TRANSPORT_PROTOCOL_VERSION,
                "decoded message must preserve version");
    expect_true(decoded.type == SYSMON_TRANSPORT_MESSAGE_TYPE_REPORT,
                "decoded message must preserve type");
    expect_true(decoded.payload_length == strlen("report:ok"),
                "decoded message must preserve payload length");
    expect_true(memcmp(decoded.payload, "report:ok", strlen("report:ok")) == 0,
                "decoded message must preserve payload contents");

    memcpy(malformed, buffer, encoded_size);
    malformed[0] = 0x00;
    rc = sysmon_transport_deserialize_message(malformed, encoded_size, &decoded);
    expect_true(rc == -2, "deserialization must fail on bad magic");

    rc = sysmon_transport_deserialize_message(buffer, encoded_size - 1, &decoded);
    expect_true(rc == -2, "deserialization must fail on truncated buffer");

    memcpy(malformed, buffer, encoded_size);
    malformed[7] = (unsigned char)(SYSMON_TRANSPORT_PROTOCOL_VERSION + 1u);
    rc = sysmon_transport_deserialize_message(malformed, encoded_size, &decoded);
    expect_true(rc == -2, "deserialization must fail on unsupported version");

    if (g_failures != 0) {
        return 1;
    }

    printf("test_transport: ok\n");
    return 0;
}
