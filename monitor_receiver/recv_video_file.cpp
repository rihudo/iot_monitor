#include "recv_video_file.hpp"
#include <string.h>
#include <unistd.h>
#include <string>

#define FILE_HEAD   "BABYCARE_START"
#define FILE_DATA   "BABYCARE_DATA"
#define FILE_END    "BABYCARE_END"

//1. START message: "BABYCARE_START" + segments number(2byte) + FILE SIZE(4byte) + FILE NAME(remaining length of message)
// 2. DATA message: "BABYCARE_DATA" + segment index(2byte) + file data(remaining length of message)
static FILE* video_file = NULL;
static std::string last_file_name;

static void handle_video_head(const char* msg, uint32_t msg_size)
{
    uint16_t segment_number = 0;
    uint32_t file_size = 0;
    memcpy(&segment_number, msg, 2);
    memcpy(&file_size, msg + 2, 4);
    std::string file_name(msg + 2 + 4, msg_size - 2 - 4);

    printf("Data head: segment number:%u file size:%u file name:%s\n", segment_number, file_size, file_name.c_str());
    if (video_file)
    {
        fclose(video_file);
        video_file = NULL;
        printf("Rmoeve incomplete file:%s\n", last_file_name.c_str());
        unlink(last_file_name.c_str());
    }
    last_file_name = file_name;
    video_file = fopen((std::string(FILE_TMP_PATH) + file_name).c_str(), "w+");
}

static void handle_video_data(const char* msg, uint32_t msg_size)
{
    static uint32_t current_buffer_size = 0;

    uint16_t segment_idx = 0;
    memcpy(&segment_idx, msg, 2);
    printf("Segment idx:%u\n", segment_idx);
    fwrite(msg + 2, msg_size - 2, 1, video_file);
}

static void handle_video_end()
{
    printf("Receive video end\n");
    fclose(video_file);
    video_file = NULL;
    rename((std::string(FILE_TMP_PATH) + last_file_name).c_str(), (std::string(FILE_PATH) + last_file_name).c_str());
}

void handle_video_segment(const char* msg, uint32_t msg_size)
{
    const char* find_head = strstr(msg, FILE_HEAD);
    if (find_head && find_head == msg)
    {
        handle_video_head(msg + sizeof(FILE_HEAD) - 1, msg_size + 1 - sizeof(FILE_HEAD));
        return;
    }

    const char* find_data = strstr(msg, FILE_DATA);
    if (find_data && find_data == msg)
    {
        handle_video_data(msg + sizeof(FILE_DATA) - 1, msg_size  + 1 - sizeof(FILE_DATA));
        return;
    }

    const char* find_end = strstr(msg, FILE_END);
    if (find_end && find_end == msg)
    {
        handle_video_end();
        return;
    }

    printf("Unknow msg:%.16s\n", msg);
}