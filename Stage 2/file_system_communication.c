/**
 * This file creates simple api that abstracts communication via file system
 * between programs to simple api like calls
 */

#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "file_system_communication.h"


bool is_initialized = false;
int data_streams_size = 0;
int started_streams = 0;
Data_Stream *data_streams;

/**
 * Function called by api users to write data to files
 */
static void _send_line(Data_Stream *context, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    vfprintf(context->file_ptr, fmt, args);

    va_end(args);
}

/**
 * Function called by api users to read data from file system
 */
static char *_read_line(Data_Stream *context, char *line_buffer, int max_count)
{
    return fgets(line_buffer, max_count, context->file_ptr);
}

/**
 * Adds more memory for more streams
 */
static int _add_data_streams(void)
{
    int old_size = data_streams_size;
    int new_size = data_streams_size + 10;

    Data_Stream *tmp = realloc(data_streams, sizeof(Data_Stream) * new_size);
    if (!tmp)
    {
        // TODO log failed to init
        return -1;
    }

    data_streams = tmp;
    data_streams_size = new_size;

    // initialize only the new chunk
    for (int i = old_size; i < new_size; ++i)
    {
        _set_new_empty_data_stream(&data_streams[i]);
    }

    return 0;
}

static void _set_new_empty_data_stream(Data_Stream *stream)
{
    stream->is_active = false;
    stream->is_first_write = true;
    stream->stream_type = READ;
    stream->on_ready = NULL;
    stream->file_ptr = NULL;
    stream->stream_name[0] = '\0';
    stream->flag_file_path[0] = '\0';
    stream->data_file_path[0] = '\0';
    stream->send_line = _send_line;
    stream->read_line = _read_line;
}

int init_data_streams()
{
    if (is_initialized)
        return 1; // TODO: add loging with warning that steam was already initialzed

    data_streams_size = 10;
    started_streams = 0;

    data_streams = (Data_Stream *)malloc(sizeof(Data_Stream) * data_streams_size); // Pre allocate space for next 10 streams
    if (!data_streams)
        return -1; // TODO log failed to init

    // initialize only the new chunk
    for (int i = 0; i < data_streams_size; ++i)
    {
        _set_new_empty_data_stream(&data_streams[i]);
    }

    is_initialized = true;
    return 0;
}

/**
 * Will safely close all data streams and return memory
 * Returns 0 on success
 */
int close_data_streams()
{
    // make sure to properly close all streams
    free(data_streams);
    data_streams = NULL;
    data_streams_size = 0;
    started_streams = 0;
    is_initialized = false;
    return 0;
}

static Data_Stream *_get_new_stream()
{
    if (started_streams >= data_streams_size)
    {
        if (_add_data_streams())
        {
            return NULL; // TODO: log failure to log
        }
    }

    // create new data stream
    int data_stream_index = started_streams++;
    Data_Stream *new_data_stream = &data_streams[data_stream_index];

    return new_data_stream;
}

bool _is_stream_name_valid(const char *stream_name, enum Stream_type stream_type)
{
    // Make sure there is no stream with the same name
    for (int i = 0; i < data_streams_size; ++i)
    {
        // will return null if stream with same name is found in the list
        if (data_streams[i].is_active && data_streams[i].stream_type == stream_type && !strcmp(data_streams[i].stream_name, stream_name))
        {
            return false; // TODO: log that stream already exists
        }
    }

    return true;
}

/**
 * Creates new data stream with specified name
 * and will invoke on_ready every time new frame can be sent
 * returns non zero if it fails
 */
int create_new_data_stream(const char *stream_name, enum Stream_type stream_type, void (*on_ready)(Data_Stream *))
{
    if (!is_initialized)
    {
        return 1; // TODO: log that we couldn't initialize the new stream
    }

    if (!_is_stream_name_valid(stream_name, stream_type))
    {
        return 1;
    }

    Data_Stream *new_data_stream = _get_new_stream();
    if (!new_data_stream)
        return 1; // TODO: log that stream failed to initialized

    // Assign the stream name, in a safe way to prevent buffer overflow
    snprintf(new_data_stream->stream_name,
             sizeof(new_data_stream->stream_name),
             "%s",
             stream_name);

    snprintf(new_data_stream->data_file_path,
             sizeof(new_data_stream->data_file_path),
             "%s",
             stream_name);
    strcat(new_data_stream->data_file_path, ".txt");

    snprintf(new_data_stream->flag_file_path,
             sizeof(new_data_stream->flag_file_path),
             "%s",
             stream_name);
    strcat(new_data_stream->flag_file_path, ".flag");

    snprintf(new_data_stream->ack_file_path,
             sizeof(new_data_stream->ack_file_path),
             "%s",
             stream_name);
    strcat(new_data_stream->ack_file_path, ".ack");

    new_data_stream->on_ready = on_ready;
    new_data_stream->stream_type = stream_type;
    new_data_stream->is_active = true;

    return 0;
}

/**
 * Invokes main action for each stream once
 * Handles opening and closing of files and setting ready flags
 */
void update_stream()
{
    if (!is_initialized || !data_streams)
        return;

    // Invoke function for every new stream
    for (int i = 0; i < data_streams_size; ++i)
    {
        if (data_streams[i].is_active && data_streams[i].on_ready != NULL)
        {
            if (data_streams[i].stream_type == WRITE)
            {
                if (_was_data_read(&data_streams[i]) || data_streams[i].is_first_write)
                {
                    data_streams[i].is_first_write = false;
                    if (_open_file_write(&data_streams[i]))
                    {
                        // TODO: log failed to open the file
                        continue;
                    }

                    data_streams[i].on_ready(&data_streams[i]);

                    _close_file(&data_streams[i]);
                    _create_flag(&data_streams[i]);
                    _remove_ack(&data_streams[i]);
                }
            }
            else if (data_streams->stream_type == READ)
            {
                if (_is_data_ready(&data_streams[i]))
                {
                    if (_open_file_read(&data_streams[i]))
                    {
                        // TODO: log failed to open the file
                        continue;
                    }

                    data_streams[i].on_ready(&data_streams[i]);

                    _close_file(&data_streams[i]);
                    _create_ack(&data_streams[i]);
                    _remove_flag(&data_streams[i]);
                }
            }
        }
    }
}

static int _is_data_ready(Data_Stream *stream)
{
    return _file_exists(stream->flag_file_path);
}

static int _was_data_read(Data_Stream *stream)
{
    return _file_exists(stream->ack_file_path);
}

/*
 * file_exists function Checks if a file exists.
 * filename is the name of the file to check.
 * file_exists return 1 if the file exists, 0 otherwise.
 */
static int _file_exists(const char *file_path){
    FILE *file = fopen(file_path, "r");
    if (file)
    {
        fclose(file);
        return 1;
    }
    return 0;
}

static void _remove_flag(Data_Stream *stream)
{
    remove(stream->flag_file_path);
}

static void _remove_ack(Data_Stream *stream)
{
    remove(stream->ack_file_path);
}

static int _open_file_write(Data_Stream *stream)
{
    stream->file_ptr = fopen(stream->data_file_path, "w");

    if (stream->file_ptr == NULL)
        return 1; // TODO log error
    return 0;
}

static int _open_file_read(Data_Stream *stream)
{
    stream->file_ptr = fopen(stream->data_file_path, "r");

    if (stream->file_ptr == NULL)
        return 1; // TODO log error
    return 0;
}

static int _close_file(Data_Stream *stream)
{
    fclose(stream->file_ptr);
    stream->file_ptr = NULL;
}

static int _create_flag(Data_Stream *stream)
{
    return _create_file(stream->flag_file_path);
}

static int _create_ack(Data_Stream *stream)
{
    return _create_file(stream->ack_file_path);
}

static int _create_file(const char *file_path)
{
    FILE *temp = fopen(file_path, "w");

    if (temp == NULL)
        return 1; // TODO log error

    fclose(temp);
    return 0;
}
