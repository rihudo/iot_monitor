#pragma once
#include <cstdint>

// Of course, you should call this function before other functions in this file. return 0 when succeeded.
int init_monitor(int cap_index);

// frame_rates: Number of frames per second. It will block
int start_monitor(uint8_t fps, uint32_t duration);

void stop_monitor();