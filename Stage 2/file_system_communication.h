#ifndef FILE_SYS_COM
#define FILE_SYS_COM

#include <stdbool.h>
#include <stdio.h>

enum Stream_type {
    READ,
    WRITE
};

typedef struct Data_Stream
{
    bool is_active;
    bool is_first_write;
    enum Stream_type stream_type;
    char stream_name[80];
    char data_file_path[84];
    char flag_file_path[85];
    char ack_file_path[84];
    FILE * file_ptr;
    void (*on_ready)(struct Data_Stream *);
    void (*send_line)(struct Data_Stream *, const char *fmt, ...);
    char * (*read_line)(struct Data_Stream *, char *, int);
} Data_Stream;

int create_new_data_stream(const char *stream_name, enum Stream_type stream_type, void (*on_ready)(Data_Stream *));
int close_data_streams();
int init_data_streams();
void update_stream();

static int _open_file_read(Data_Stream *stream);
static int _open_file_write(Data_Stream *stream);
static int _close_file(Data_Stream *stream);
static void _set_new_empty_data_stream(Data_Stream *stream);
static void _remove_flag(Data_Stream *stream);
static int _create_flag(Data_Stream *stream);
static int _create_ack(Data_Stream *stream);
static void _remove_ack(Data_Stream *stream);
static int _is_data_ready(Data_Stream *stream);
static int _was_data_read(Data_Stream *stream);
static int _create_file(const char *file_path);
static int _file_exists(const char *file_path);

#endif