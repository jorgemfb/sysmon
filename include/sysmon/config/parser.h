#ifndef SYSMON_CONFIG_PARSER_H
#define SYSMON_CONFIG_PARSER_H

typedef struct sysmon_config_file {
    const char *mode;
} sysmon_config_file_t;

int sysmon_config_parse_file(const char *path, sysmon_config_file_t *out);

#endif
