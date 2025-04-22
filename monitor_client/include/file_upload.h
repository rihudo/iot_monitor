#pragma once

// This will block. Call the function in a separate thread
void start_upload_file();

void stop_upload_file();

// New file is ready for upload
void notify_new_file(const char* file_name);