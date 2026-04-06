#include "sysmon/report/renderer.h"

#include "sysmon/report/markdown.h"
#include "sysmon/report/template.h"

int sysmon_report_render_markdown(const sysmon_snapshot_t *snapshot,
                                  char *buffer,
                                  size_t buffer_size,
                                  size_t *written_out) {
    char timestamp[64];

    if (snapshot == NULL || buffer == NULL || buffer_size == 0) {
        return -1;
    }

    (void)sysmon_report_format_timestamp(timestamp, sizeof(timestamp));
    return sysmon_report_markdown_build(snapshot,
                                        sysmon_report_default_title(),
                                        timestamp,
                                        buffer,
                                        buffer_size,
                                        written_out);
}
