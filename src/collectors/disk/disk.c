#include "sysmon/collectors/disk/disk.h"

#include <stddef.h>
#include <sys/statvfs.h>

int sysmon_collect_disk(sysmon_snapshot_t *snapshot) {
    struct statvfs fs;
    uint64_t block_size;
    uint64_t total_kib;
    uint64_t available_kib;
    uint64_t used_kib;

    if (snapshot == NULL) {
        return -1;
    }

    if (statvfs("/", &fs) != 0) {
        return -2;
    }

    block_size = fs.f_frsize != 0 ? (uint64_t)fs.f_frsize : (uint64_t)fs.f_bsize;
    total_kib = ((uint64_t)fs.f_blocks * block_size) / 1024ULL;
    available_kib = ((uint64_t)fs.f_bavail * block_size) / 1024ULL;

    if (total_kib == 0) {
        return -3;
    }

    if (available_kib > total_kib) {
        available_kib = total_kib;
    }

    used_kib = total_kib - available_kib;

    snapshot->disk_total_kib = total_kib;
    snapshot->disk_available_kib = available_kib;
    snapshot->disk_used_kib = used_kib;
    snapshot->sample_count++;

    return 0;
}
