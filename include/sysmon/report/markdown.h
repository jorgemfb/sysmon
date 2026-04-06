#ifndef INCLUDE_SYSMON_REPORT_MARKDOWN_H
#define INCLUDE_SYSMON_REPORT_MARKDOWN_H

#include <stddef.h>

#include "sysmon/core/snapshot.h"

int sysmon_report_markdown_build(const sysmon_snapshot_t *snapshot,
                                 const char *title,
                                 const char *timestamp,
                                 char *buffer,
                                 size_t buffer_size,
                                 size_t *written_out);

#endif
