#include "file_upload.h"
#include <mutex>
#include <condition_variable>
#include <list>
#include <string>
#include <data_transmit.h>
#include <unistd.h>

#define KEEP_ALIVE_FILE_NAME "keep_alive"

static std::mutex upload_mtx;
static std::condition_variable upload_cv;
static std::list<std::string> ready_files;
static bool is_upload_stoped = false;

void start_upload_file()
{
    printf("start uploading file\n");
    for(;!is_upload_stoped;)
    {
        if (!ready_files.empty())
        {
            if (ready_files.front() == KEEP_ALIVE_FILE_NAME)
            {
                printf("Keep alive\n");
                keep_alive();
            }
            else if (!ready_files.front().empty())
            {
                send_file(ready_files.front().c_str());
                unlink(ready_files.front().c_str());
                printf("File(%s) upload completed\n", ready_files.front().c_str());
            }
            ready_files.pop_front();
            continue;
        }
        std::unique_lock<std::mutex> lk(upload_mtx);
        upload_cv.wait(lk);
    }
    printf("Stop uploading file\n");
}

void stop_upload_file()
{
    is_upload_stoped = true;
}

void notify_new_file(const char* file_name)
{
    std::unique_lock<std::mutex> lk(upload_mtx);
    ready_files.push_back(file_name);
    upload_cv.notify_all();
}