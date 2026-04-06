#include "sysmon/core/app.h"
#include "sysmon/transport/message.h"
#include "sysmon/transport/serializer.h"
#include "sysmon/transport/socket.h"

#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

int main(void) {
    sysmon_app_t app;
    pid_t child_pid;
    int status = 0;
    const unsigned short port = (unsigned short)(21000u + ((unsigned)getpid() % 10000u));

    if (sysmon_app_init(&app) != 0) {
        fprintf(stderr, "test_server_mode: app init failed\n");
        return 1;
    }

    app.context.config.mode = "server";
    app.context.config.default_interval_ms = 10;

    if (sysmon_app_register_defaults(&app) != 0) {
        fprintf(stderr, "test_server_mode: register defaults failed\n");
        sysmon_app_shutdown(&app);
        return 1;
    }

    child_pid = fork();
    if (child_pid < 0) {
        fprintf(stderr, "test_server_mode: fork failed\n");
        sysmon_app_shutdown(&app);
        return 1;
    }

    if (child_pid == 0) {
        int client_fd = sysmon_transport_socket_create_tcp();
        unsigned char header[16];
        unsigned char frame[4096];
        sysmon_transport_message_t decoded;
        uint32_t payload_len_u32;
        size_t payload_len;
        int attempts = 600;
        struct timespec delay;

        delay.tv_sec = 0;
        delay.tv_nsec = 20000000L;

        while (attempts-- > 0) {
            if (client_fd >= 0 &&
                sysmon_transport_socket_connect(client_fd, "127.0.0.1", port) == 0) {
                break;
            }
            if (client_fd >= 0) {
                (void)sysmon_transport_socket_close(client_fd);
            }
            client_fd = sysmon_transport_socket_create_tcp();
            (void)nanosleep(&delay, NULL);
        }

        if (client_fd < 0) {
            _exit(10);
        }
        if (attempts <= 0) {
            (void)sysmon_transport_socket_close(client_fd);
            _exit(11);
        }
        if (sysmon_transport_socket_receive_all(client_fd, header, sizeof(header)) != 0) {
            (void)sysmon_transport_socket_close(client_fd);
            _exit(12);
        }
        payload_len_u32 = ((uint32_t)header[12] << 24) |
                          ((uint32_t)header[13] << 16) |
                          ((uint32_t)header[14] << 8) |
                          (uint32_t)header[15];
        payload_len = (size_t)payload_len_u32;
        if (payload_len > (sizeof(frame) - sizeof(header))) {
            (void)sysmon_transport_socket_close(client_fd);
            _exit(13);
        }
        memcpy(frame, header, sizeof(header));
        if (payload_len > 0 &&
            sysmon_transport_socket_receive_all(client_fd, frame + sizeof(header), payload_len) != 0) {
            (void)sysmon_transport_socket_close(client_fd);
            _exit(14);
        }
        if (sysmon_transport_deserialize_message(frame, sizeof(header) + payload_len, &decoded) != 0 ||
            decoded.type != SYSMON_TRANSPORT_MESSAGE_TYPE_REPORT ||
            decoded.payload_length == 0) {
            (void)sysmon_transport_socket_close(client_fd);
            _exit(15);
        }
        (void)sysmon_transport_socket_close(client_fd);
        _exit(0);
    }

    {
        int run_rc = sysmon_app_run_server(&app, "127.0.0.1", port);
        if (run_rc != 0) {
            fprintf(stderr, "test_server_mode: app server mode run failed (rc=%d)\n", run_rc);
            sysmon_app_shutdown(&app);
            return 1;
        }
    }

    if (waitpid(child_pid, &status, 0) != child_pid || !WIFEXITED(status) || WEXITSTATUS(status) != 0) {
        fprintf(stderr, "test_server_mode: child validation failed\n");
        sysmon_app_shutdown(&app);
        return 1;
    }

    sysmon_app_shutdown(&app);
    printf("test_server_mode: ok\n");
    return 0;
}
