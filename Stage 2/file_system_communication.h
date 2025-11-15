#ifndef FILE_SYS_COM
#define FILE_SYS_COM

#include <stdbool.h>
#include <stdio.h>

#define MAX_NAME_LENGTH 80

enum Stream_type {
    READ,
    WRITE
};

typedef struct Data_Stream
{
    bool is_active;
    bool is_first_write;
    enum Stream_type stream_type;
    char stream_name[MAX_NAME_LENGTH];
    char data_file_path[MAX_NAME_LENGTH + 4]; // stream name + .txt
    char flag_file_path[MAX_NAME_LENGTH + 5]; // stream name + .flag
    char ack_file_path[MAX_NAME_LENGTH + 4]; // stream name + .ack
    FILE * file_ptr;
    void (*on_ready)(struct Data_Stream *);
    void (*send_line)(struct Data_Stream *, const char *fmt, ...);
    char * (*read_line)(struct Data_Stream *, char *, int);
} Data_Stream;

int create_new_data_stream(const char *stream_name, enum Stream_type stream_type, void (*on_ready)(Data_Stream *));
int close_data_streams();
int init_data_streams();
void update_stream();

#endif