#ifndef FILE_SYS_COM
#define FILE_SYS_COM

int create_new_data_stream(const char *stream_name, void (*on_ready)());
int close_data_streams();
int init_data_streams();
void update_stream();

#endif