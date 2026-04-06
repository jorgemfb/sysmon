#include "sysmon/collectors/filesystem/filesystem.h"

#include <mntent.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <sys/statvfs.h>

static int sysmon_filesystem_find_mountpoint(const sysmon_filesystem_entry_t *entries, size_t count, const char *mountpoint) {
    size_t i;

    for (i = 0; i < count; ++i) {
        if (strcmp(entries[i].mountpoint, mountpoint) == 0) {
            return 1;
        }
    }

    return 0;
}

int sysmon_collect_filesystem(sysmon_snapshot_t *snapshot) {
    FILE *mnt_file;
    struct mntent *mnt;
    size_t count = 0;

    if (snapshot == NULL) {
        return -1;
    }

    snapshot->filesystem_count = 0;

    mnt_file = setmntent("/proc/mounts", "r");
    if (mnt_file == NULL) {
        return -2;
    }

    while ((mnt = getmntent(mnt_file)) != NULL) {
        struct statvfs fs;
        uint64_t block_size;
        uint64_t total_kib;
        uint64_t available_kib;
        uint64_t used_kib;

        if (mnt->mnt_dir == NULL || mnt->mnt_dir[0] == '\0') {
            continue;
        }

        if (sysmon_filesystem_find_mountpoint(snapshot->filesystem_entries, count, mnt->mnt_dir)) {
            continue;
        }

        if (statvfs(mnt->mnt_dir, &fs) != 0) {
            continue;
        }

        block_size = fs.f_frsize != 0 ? (uint64_t)fs.f_frsize : (uint64_t)fs.f_bsize;
        total_kib = ((uint64_t)fs.f_blocks * block_size) / 1024ULL;
        available_kib = ((uint64_t)fs.f_bavail * block_size) / 1024ULL;

        if (total_kib == 0) {
            continue;
        }

        if (available_kib > total_kib) {
            available_kib = total_kib;
        }

        used_kib = total_kib - available_kib;

        (void)snprintf(snapshot->filesystem_entries[count].mountpoint,
                       sizeof(snapshot->filesystem_entries[count].mountpoint),
                       "%s", mnt->mnt_dir);
        (void)snprintf(snapshot->filesystem_entries[count].fs_type,
                       sizeof(snapshot->filesystem_entries[count].fs_type),
                       "%s", mnt->mnt_type != NULL ? mnt->mnt_type : "unknown");
        snapshot->filesystem_entries[count].total_kib = total_kib;
        snapshot->filesystem_entries[count].available_kib = available_kib;
        snapshot->filesystem_entries[count].used_kib = used_kib;

        count++;
        if (count >= SYSMON_FILESYSTEM_MAX_ENTRIES) {
            break;
        }
    }

    endmntent(mnt_file);

    snapshot->filesystem_count = count;
    snapshot->sample_count++;

    return 0;
}
