#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <MQTTClient.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "recv_video_file.hpp"

#define TOPIC       "/BABYCARE/VIODE"

#define BUFFER_SIZE (4 * 1024)
static bool is_stoped = false;

void receive_msg(MessageData* md)
{
    handle_video_segment((char*)md->message->payload, (int)md->message->payloadlen);
}

void defaultMessageHandler(MessageData* md)
{
    handle_video_segment((char*)md->message->payload, (int)md->message->payloadlen);
}

void handle_sigint(int sig)
{
    printf("receive signal:%d\n", sig);
    is_stoped = true;
}

int main(int argc, char* argv[])
{
    if (argc < 3)
    {
        printf("Usage:\n    monitor_receiver <server address> <client id> [service port: default 1883]\n");
        return 1;
    }

    if (0 != access(FILE_PATH, F_OK))
    {
        mkdir(FILE_PATH, 0777);
    }

    if (0 != access(FILE_TMP_PATH, F_OK))
    {
        mkdir(FILE_TMP_PATH, 0777);
    }

    signal(SIGINT, handle_sigint);

    int rc = 0;
    unsigned char buf[BUFFER_SIZE] = {0};
    unsigned char readbuf[BUFFER_SIZE] = {0};

	Network n;
    char address_str[32] = {0};
    strncpy(address_str, argv[1], sizeof(address_str));
	NetworkInit(&n);
    int service_port = argc > 3 ? atoi(argv[3]) : 1883;
	NetworkConnect(&n, address_str, service_port);

	MQTTClient c;
	MQTTClientInit(&c, &n, 1000, buf, BUFFER_SIZE, readbuf, BUFFER_SIZE);
    c.defaultMessageHandler = defaultMessageHandler;

	MQTTPacket_connectData data = MQTTPacket_connectData_initializer;       
	data.willFlag = 0;
	data.MQTTVersion = 3;
    char client_id[32] = {0};
    strncpy(client_id, argv[2], sizeof(client_id));
	data.clientID.cstring = client_id;
	data.keepAliveInterval = 60;
	data.cleansession = 1;
	
	rc = MQTTConnect(&c, &data);
    printf("MQTTConnect() rc:%d\n", rc);
    rc = MQTTSubscribe(&c, TOPIC, QOS1, receive_msg);
    printf("subscribe rc:%d\n", rc);
    while (!is_stoped)
    {
        MQTTYield(&c, 1000);
    }
    printf("exit\n");
    MQTTDisconnect(&c);
    return 0;
}