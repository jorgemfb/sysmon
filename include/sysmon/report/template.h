#ifndef INCLUDE_SYSMON_REPORT_TEMPLATE_H
#define INCLUDE_SYSMON_REPORT_TEMPLATE_H

#include <stddef.h>

const char *sysmon_report_default_title(void);
int sysmon_report_format_timestamp(char *buffer, size_t buffer_size);

#endif
