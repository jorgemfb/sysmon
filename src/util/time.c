#include "sysmon/util/time.h"

#include <errno.h>
#include <time.h>

uint64_t sysmon_time_now_ms(void) {
    struct timespec ts;

    if (clock_gettime(CLOCK_MONOTONIC, &ts) != 0) {
        return 0;
    }

    return ((uint64_t)ts.tv_sec * 1000ULL) + ((uint64_t)ts.tv_nsec / 1000000ULL);
}

int sysmon_time_sleep_ms(uint64_t ms) {
    struct timespec req;
    struct timespec rem;

    req.tv_sec = (time_t)(ms / 1000ULL);
    req.tv_nsec = (long)((ms % 1000ULL) * 1000000ULL);

    while (nanosleep(&req, &rem) != 0) {
        if (errno != EINTR) {
            return -1;
        }

        req = rem;
    }

    return 0;
}
