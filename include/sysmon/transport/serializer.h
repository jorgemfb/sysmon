#ifndef INCLUDE_SYSMON_TRANSPORT_SERIALIZER_H
#define INCLUDE_SYSMON_TRANSPORT_SERIALIZER_H

#include <stddef.h>

#include "sysmon/transport/message.h"

int sysmon_transport_serialize_message(const sysmon_transport_message_t *message,
                                       unsigned char *buffer,
                                       size_t buffer_size,
                                       size_t *written_out);
int sysmon_transport_deserialize_message(const unsigned char *buffer,
                                         size_t buffer_size,
                                         sysmon_transport_message_t *message_out);

#endif
