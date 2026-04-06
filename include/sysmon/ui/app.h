#ifndef INCLUDE_SYSMON_UI_APP_H
#define INCLUDE_SYSMON_UI_APP_H

#include <stddef.h>
#include <stdio.h>
#include <stdint.h>

#include "sysmon/core/snapshot.h"

int sysmon_ui_render_snapshot(FILE *out, const sysmon_snapshot_t *snapshot, const char *status);
int sysmon_ui_render_remote_report(FILE *out,
                                   const char *remote_host,
                                   uint16_t remote_port,
                                   const char *payload,
                                   size_t payload_length);

#endif
