#ifndef FILE_SYS_COM
#define FILE_SYS_COM

#include <stdbool.h>
#include <stdio.h>

#define MAX_NAME_LENGTH 80

// File Extensions
#define DATA_FILE_EXTENSION ".txt"
#define FLAG_FILE_EXTENSION ".flag"
#define ACK_FILE_EXTENSION ".ack"

#define STRLEN_LITERAL(x) (sizeof(x) - 1)

enum Stream_type
{
    READ_ONLY_STREAM,
    WRITE_ONLY_STREAM
};

typedef struct Data_Stream
{
    bool is_active;
    bool is_first_write;
    enum Stream_type stream_type;
    char stream_name[MAX_NAME_LENGTH];
    char data_file_path[MAX_NAME_LENGTH + STRLEN_LITERAL(DATA_FILE_EXTENSION)]; // stream name + .txt
    char flag_file_path[MAX_NAME_LENGTH + STRLEN_LITERAL(FLAG_FILE_EXTENSION)]; // stream name + .flag
    char ack_file_path[MAX_NAME_LENGTH + STRLEN_LITERAL(ACK_FILE_EXTENSION)];   // stream name + .ack
    FILE *data_file_ptr;
    void (*on_ready)(struct Data_Stream *);
    /**
     * Function called by api users to write data to file system. Behaves same as fprintf.
     * \param context contains all the function calls and provides necessary context for the function to be executed on the right files
     * \param fmt format string
     * \param ... parameters specified in format string
     */
    void (*send_line)(struct Data_Stream *, const char *fmt, ...);
    /**
     * Function called by api users to read data from file system. Behaves same as fgets. writes data to buffer
     * until buffer size is reached or until EOF is reached
     * \param context contains all the function calls and provides necessary context for the function to be executed on the right files
     * \param line_buffer string to which data will be written
     * \param max_count size of the buffer
     * \return returns null of EOF was reached
     */
    char *(*read_line)(struct Data_Stream *, char *, int);
} Data_Stream;

int create_new_data_stream(const char *stream_name, enum Stream_type stream_type, void (*on_ready)(Data_Stream *));
int close_data_streams();
int init_data_streams();
void update_stream();

#endif