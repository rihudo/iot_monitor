#include "monitor.h"
#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>
#include <sys/epoll.h>
#include <time.h>
#include <sys/timerfd.h>
#include <stdio.h>
#include <thread>
#include <unistd.h>
#include <atomic>
#include "file_upload.h"

static cv::Mat src;
static cv::VideoCapture cap;
static cv::VideoWriter writer;
static bool is_stoped = false;
static std::atomic<bool> it_is_time = false;
#define KEEP_ALIVE_FILE_NAME "keep_alive"

int init_monitor(int cap_index)
{
    cap.open(cap_index);
    // check if we succeeded
    if (!cap.isOpened())
    {
        printf("ERROR! Unable to open camera\n");
        return -1;
    }

    return 0;
}

int start_monitor(uint8_t fps, uint32_t duration)
{
    double width = cap.get(cv::CAP_PROP_FRAME_WIDTH);
    double height = cap.get(cv::CAP_PROP_FRAME_HEIGHT);
    double real_fps = cap.get(cv::CAP_PROP_FPS);
    printf("width:%f height:%f fps:%f\n", width, height, real_fps);
    if (fps > real_fps)
    {
        printf("fps is greater than camera fps. Set fps to %f\n", real_fps);
        fps = real_fps;
    }
    is_stoped = false;
    cap >> src;
    // check if we succeeded
    if (src.empty())
    {
        printf("ERROR! blank frame grabbed\n");
        return -1;
    }

    // Start timer
    int timer_fd = timerfd_create(CLOCK_MONOTONIC, 0);
    if (-1 == timer_fd)
    {
        printf("timerfd_create() failed\n");
        return -2;
    }

    int epoll_id = epoll_create1(0);
    if (-1 == epoll_id)
    {
        close(timer_fd);
        printf("epoll_create1() failed\n");
        return -3;
    }

    struct epoll_event timer_event;
    memset(&timer_event, 0, sizeof(timer_event));
    timer_event.events = EPOLLIN;
    timer_event.data.fd = timer_fd;
    if (0 != epoll_ctl(epoll_id, EPOLL_CTL_ADD, timer_fd, &timer_event))
    {
        close(timer_fd);
        close(epoll_id);
        printf("epoll_ctl failed\n");
        return -4;
    }
    struct epoll_event events[1] = {0};
    std::thread timer_thd([&]{
        uint64_t buf;
        uint32_t heart_count = 0, count = 0;;
        for (;!is_stoped;)
        {
            if (epoll_wait(epoll_id, events, 1, -1) > 0)
            {
                read(timer_fd, &buf, sizeof(buf));
                if (++heart_count == 10)
                {
                    heart_count = 0;
                    notify_new_file(KEEP_ALIVE_FILE_NAME);
                }
                if (++count == duration)
                {
                    count = 0;
                    it_is_time = true;
                }
                printf("lhood notify %u\n", count);
            }
        }
    });
    timer_thd.detach();

    struct itimerspec time_value;
    memset(&time_value, 0, sizeof(time_value));
    time_value.it_value.tv_sec = time_value.it_interval.tv_sec = 1;
    timerfd_settime(timer_fd, 0, &time_value, NULL);

    // Start recording video
    char file_name[64] = {0};
    uint32_t timer_count = 0;
    for (;!is_stoped;)
    {
        snprintf(file_name, sizeof(file_name), "./%ld.avi", time(NULL));
        printf("File name:/%s\n", file_name);
        writer.open(file_name, cv::VideoWriter::fourcc('M', 'J', 'P', 'G'), fps, src.size(), src.type() == CV_8UC3);
        if (!writer.isOpened())
        {
            printf("Failed to open video writer\n");
            continue;
        }
        while (!is_stoped && !it_is_time)
        {
            auto frame_start = std::chrono::steady_clock::now();
            cap.read(src);
            writer.write(src);
            auto frame_end = std::chrono::steady_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(frame_end - frame_start);
            std::this_thread::sleep_for(std::chrono::microseconds(1000 * 1000 / fps) - elapsed);
        }
        it_is_time = false;
        writer.release();
        notify_new_file(file_name);
    }
    return 0;
}

void stop_monitor()
{
    is_stoped = true;
}