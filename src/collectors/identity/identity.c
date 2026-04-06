#include "sysmon/collectors/identity/identity.h"

#include <pwd.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/sysinfo.h>
#include <sys/utsname.h>
#include <unistd.h>

static void sysmon_identity_store_text(char *dst, size_t dst_size, const char *value) {
    const char *src = value;

    if (src == NULL || src[0] == '\0') {
        src = "unknown";
    }

    (void)snprintf(dst, dst_size, "%s", src);
}

static void sysmon_identity_set_hostname(sysmon_snapshot_t *snapshot) {
    struct utsname uts;

    if (gethostname(snapshot->identity_hostname, sizeof(snapshot->identity_hostname)) == 0) {
        snapshot->identity_hostname[sizeof(snapshot->identity_hostname) - 1] = '\0';
        return;
    }

    if (uname(&uts) == 0) {
        sysmon_identity_store_text(snapshot->identity_hostname, sizeof(snapshot->identity_hostname), uts.nodename);
        return;
    }

    sysmon_identity_store_text(snapshot->identity_hostname, sizeof(snapshot->identity_hostname), NULL);
}

static void sysmon_identity_set_username(sysmon_snapshot_t *snapshot) {
    struct passwd *pw = getpwuid(getuid());
    const char *user = NULL;

    if (pw != NULL && pw->pw_name != NULL && pw->pw_name[0] != '\0') {
        user = pw->pw_name;
    } else {
        user = getenv("USER");
        if (user == NULL || user[0] == '\0') {
            user = getenv("LOGNAME");
        }
    }

    sysmon_identity_store_text(snapshot->identity_username, sizeof(snapshot->identity_username), user);
}

static void sysmon_identity_set_kernel(sysmon_snapshot_t *snapshot) {
    struct utsname uts;

    if (uname(&uts) == 0) {
        snprintf(snapshot->identity_kernel, sizeof(snapshot->identity_kernel), "%s %s", uts.sysname, uts.release);
    } else {
        sysmon_identity_store_text(snapshot->identity_kernel, sizeof(snapshot->identity_kernel), NULL);
    }
}

static void sysmon_identity_set_uptime(sysmon_snapshot_t *snapshot) {
    struct sysinfo info;
    FILE *fp;
    double uptime_seconds = 0.0;

    snapshot->identity_uptime_seconds = 0;

    if (sysinfo(&info) == 0 && info.uptime >= 0) {
        snapshot->identity_uptime_seconds = (uint64_t)info.uptime;
        return;
    }

    fp = fopen("/proc/uptime", "r");
    if (fp == NULL) {
        return;
    }

    if (fscanf(fp, "%lf", &uptime_seconds) == 1 && uptime_seconds >= 0.0) {
        snapshot->identity_uptime_seconds = (uint64_t)uptime_seconds;
    }

    fclose(fp);
}

int sysmon_collect_identity(sysmon_snapshot_t *snapshot) {
    if (snapshot == NULL) {
        return -1;
    }

    sysmon_identity_set_hostname(snapshot);
    sysmon_identity_set_username(snapshot);
    sysmon_identity_set_kernel(snapshot);
    sysmon_identity_set_uptime(snapshot);
    snapshot->sample_count++;

    return 0;
}
