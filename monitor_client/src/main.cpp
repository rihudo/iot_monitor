#include "data_transmit.h"
#include "monitor.h"
#include <stdio.h>
#include <signal.h>
#include <thread>
#include <stdlib.h>
#include <file_upload.h>


void handle_signal(int sig)
{
    printf("Got signal:%d\n", sig);
    stop_monitor();
    stop_upload_file();
    notify_new_file("");
}

int main(int argc, char** argv)
{
    signal(SIGINT, handle_signal);

    if (argc < 5)
    {
        printf("Usage:\n    monitor_client <server address> <client id> <fps> <single video file length:seconds> [service port: default 1883] [camera_id: default 0]\n");
        return -1;
    }

    int port = argc > 5 ? atoi(argv[5]) : 1883;
    // Initialize mqtt connection
    init_mqtt_client(argv[1], argv[2], port);

    // Run thread of file upload
    std::thread upload_thd(start_upload_file);
    upload_thd.detach();

    // Initialize monitor
    int cap_idx = argc > 6 ? atoi(argv[6]) : 0;
    init_monitor(cap_idx);

    // Start to monitor
    start_monitor(strtoul(argv[3], NULL, 10), strtoul(argv[4], NULL, 10));
    printf("Stop monitoring\n");
    close_mqtt_client();
}