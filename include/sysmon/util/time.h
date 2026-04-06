#ifndef SYSMON_UTIL_TIME_H
#define SYSMON_UTIL_TIME_H

#include <stdint.h>

uint64_t sysmon_time_now_ms(void);
int sysmon_time_sleep_ms(uint64_t ms);

#endif
