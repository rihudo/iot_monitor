#pragma once

// You should call this function before any other functions. Return 0 when succeeded
int init_mqtt_client(const char* server_address, const char* client_id, int port);

void close_mqtt_client();

int send_data(const char* data, unsigned data_size);

int send_file(const char* file_name);

void keep_alive();