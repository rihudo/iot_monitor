#include "data_transmit.h"
#include "MQTTClient.h"
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <cstdint>
 #include <stdlib.h>
#include <cstring>
#include <libgen.h>

static unsigned char send_buffer[4 * 1024] = {0};
static unsigned char read_buffer[4 * 1024] = {0};
static char playload_str[4 * 1024] = {0};
static char server_add_str[32] = {0};
static char client_id_str[32] = {0};
static Network n;
static MQTTClient c;
static bool is_mqtt_stoped = false;
static MQTTPacket_connectData data = MQTTPacket_connectData_initializer;

#define TOPIC_NAME "/BABYCARE/VIODE"
#define FILE_SEGMENT_SIZE (3*1024)
#define FILE_HEAD   "BABYCARE_START"
#define FILE_DATA   "BABYCARE_DATA"
#define FILE_END    "BABYCARE_END"

int init_mqtt_client(const char* server_address, const char* client_id, int port)
{
    int rc = 0;
    strncpy(server_add_str, server_address, sizeof(server_add_str));
	NetworkInit(&n);
	NetworkConnect(&n, server_add_str, port);
    MQTTClientInit(&c, &n, 1000, send_buffer, 4 * 1024, read_buffer, 4 * 1024);

    strncpy(client_id_str, client_id, sizeof(client_id_str));
	data.willFlag = 0;
	data.MQTTVersion = 3;
	data.clientID.cstring = client_id_str;
	data.keepAliveInterval = 60;
	data.cleansession = 1;
	
	rc = MQTTConnect(&c, &data);
    printf("MQTTConnect return:%d\n", rc);
    return rc;
}

void close_mqtt_client()
{
    is_mqtt_stoped = true;
    MQTTDisconnect(&c);
}

int send_data(const char* data, unsigned data_size)
{
    if (data_size > 4 * 1024)
    {
        printf("data size is too long(> 4 * 1024)\n");
        return -1;
    }

    MQTTMessage pubmsg;
    memset(&pubmsg, 0, sizeof(pubmsg));
    memset(playload_str, 0, sizeof(playload_str));

    memcpy(playload_str, data, data_size);
    pubmsg.payload = playload_str;
    pubmsg.payloadlen = data_size;
    pubmsg.qos = QOS1;
    pubmsg.retained = 0;
    pubmsg.dup = 0;
    return MQTTPublish(&c, TOPIC_NAME, &pubmsg);
}

int send_file(const char* file_name)
{
    if (!file_name) return -1;

    int fd = open(file_name, O_RDONLY);
    if (-1 == fd)
    {
        printf("Can't open file:%s\n", file_name);
        return -2;
    }

    struct stat st;
    if (-1 == fstat(fd, &st))
    {
        close(fd);
        printf("fstat() failed\n");
        return -3;
    }
    char pack_buffer[64] = {0};
    // Send file head
    memcpy(pack_buffer, FILE_HEAD, sizeof(FILE_HEAD) - 1);
    uint16_t segment_number = st.st_size % FILE_SEGMENT_SIZE != 0 ? st.st_size / FILE_SEGMENT_SIZE + 1 : st.st_size / FILE_SEGMENT_SIZE;
    memcpy(pack_buffer + sizeof(FILE_HEAD) - 1, &segment_number, 2);
    memcpy(pack_buffer + sizeof(FILE_HEAD) + 1, &st.st_size, 4);
    char* file_name_str = strdup(file_name);
    char* name = basename(file_name_str);
    size_t name_size = strlen(name);
    memcpy(pack_buffer + sizeof(FILE_HEAD) + 1 + 4, name, name_size);
    free(file_name_str);
    uint8_t retry_times = 0;
    int publish_ret = 0;
    while (0 != (publish_ret = send_data(pack_buffer, sizeof(FILE_HEAD) -1 + 2 + 4 + name_size)))
    {
        if (++retry_times > 3)
        {
            printf("Failed to send file head. Give up this transmission\n");
            close(fd);
            return -4;
        }
        printf("Try to send data head again:%u ret:%d\n", retry_times, publish_ret);
    }

    retry_times = 0;
    // Send file data
    size_t data_idx = 0;
    void* file_data = mmap(nullptr, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    char* send_buff = (char*)malloc(FILE_SEGMENT_SIZE + sizeof(FILE_DATA) + 1);
    memcpy(send_buff, FILE_DATA, sizeof(FILE_DATA) - 1);
    for (uint16_t idx = 0; data_idx < st.st_size; ++idx)
    {
        size_t send_size = (st.st_size - data_idx) < FILE_SEGMENT_SIZE ? st.st_size - data_idx : FILE_SEGMENT_SIZE;
        memcpy(send_buff + sizeof(FILE_DATA) - 1, &idx, 2);
        memcpy(send_buff + sizeof(FILE_DATA) + 1, (char*)file_data + data_idx, send_size);
        while (retry_times <= 3 && 0 != (publish_ret = send_data(send_buff, send_size + sizeof(FILE_DATA) + 1)))
        {
            ++retry_times;
            printf("Try to send file data again:%u ret:%d\n", retry_times, publish_ret);   
        }
        if (retry_times > 3)
        {
            printf("Failed to send file data. Give up this transmission\n");
            break;
        }
        data_idx += send_size;
    }
    retry_times = 0;
    // Send file end

    while (retry_times <= 3 && 0 != (publish_ret = send_data(FILE_END, sizeof(FILE_END) - 1)))
    {
        ++retry_times;
        printf("Try to send data end again:%u ret:%d\n", retry_times, publish_ret);
    }
    if (retry_times > 3)
    {
        printf("Send data send failed\n");
    }

    close(fd);
    munmap(file_data, st.st_size);
    return 0;
}

void keep_alive()
{
    MQTTYield(&c, 500);
}