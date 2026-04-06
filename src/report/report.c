#include "sysmon/report/report.h"

#include "sysmon/report/renderer.h"

#include <stddef.h>

int sysmon_report_generate(sysmon_snapshot_t *snapshot) {
    size_t written = 0;
    int rc;

    if (snapshot == NULL) {
        return -1;
    }

    snapshot->report_markdown[0] = '\0';
    snapshot->report_markdown_length = 0;
    snapshot->report_markdown_generated = 0;

    rc = sysmon_report_render_markdown(snapshot,
                                       snapshot->report_markdown,
                                       sizeof(snapshot->report_markdown),
                                       &written);
    if (rc != 0) {
        return rc;
    }

    snapshot->report_markdown_length = written;
    snapshot->report_markdown_generated = 1;
    return 0;
}
