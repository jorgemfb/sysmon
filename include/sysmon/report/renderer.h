#ifndef INCLUDE_SYSMON_REPORT_RENDERER_H
#define INCLUDE_SYSMON_REPORT_RENDERER_H

#include <stddef.h>

#include "sysmon/core/snapshot.h"

int sysmon_report_render_markdown(const sysmon_snapshot_t *snapshot,
                                  char *buffer,
                                  size_t buffer_size,
                                  size_t *written_out);

#endif
