#ifndef FILE_SYS_COM
#define FILE_SYS_COM

#include <stdbool.h>
#include <stdio.h>

typedef struct Data_Stream
{
    bool is_active;
    char stream_name[80];
    char data_file_path[84];
    char flag_file_path[85];
    FILE * file_ptr;
    void (*on_ready)(struct Data_Stream *);
    void (*send_line)(struct Data_Stream * , const char *);
} Data_Stream;

int create_new_data_stream(const char *stream_name, void (*on_ready)(Data_Stream *));
int close_data_streams();
int init_data_streams();
void update_stream();

#endif