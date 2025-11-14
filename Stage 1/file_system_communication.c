/**
 * This file creates simple api that abstracts communication via file system
 * between programs to simple api like calls
 */

#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "file_system_communication.h"

static int _open_file(Data_Stream * stream);
static int _close_file(Data_Stream * stream);
static int _create_flag(Data_Stream * stream);
static void _set_new_empty_data_stream(Data_Stream * stream);

bool is_initialized = false;
int data_streams_size = 0;
int started_streams = 0;
Data_Stream *data_streams;

/**
 * 
 */
static void _send_line(Data_Stream * context, const char * line){
    fprintf(context->file_ptr, line);
}

/**
 * Adds more memory for more streams
 */
static int _add_data_streams(void)
{
    int old_size = data_streams_size;
    int new_size = data_streams_size + 10;

    Data_Stream *tmp = realloc(data_streams, sizeof(Data_Stream) * new_size);
    if (!tmp) {
        // TODO log failed to init
        return -1;
    }

    data_streams = tmp;
    data_streams_size = new_size;

    // initialize only the new chunk
    for (int i = old_size; i < new_size; ++i) {
        _set_new_empty_data_stream(&data_streams[i]);
    }

    return 0;
}

static void _set_new_empty_data_stream(Data_Stream * stream){
        stream->is_active = false;
        stream->on_ready = NULL;
        stream->file_ptr = NULL;
        stream->stream_name[0] = '\0';
        stream->flag_file_path[0] = '\0';
        stream->data_file_path[0] = '\0';
        stream->send_line = _send_line;
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
    for (int i = 0; i < data_streams_size; ++i) {
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

/**
 * Creates new data stream with specified name
 * and will invoke on_ready every time new frame can be sent
 * returns non zero if it fails
 */
int create_new_data_stream(const char *stream_name, void (*on_ready)(Data_Stream *))
{
    if (!is_initialized)
    {
        return 1; // TODO: log that we couldnt initilize the new stream
    }

    // Make sure there is no stream with the same name
    for (int i = 0; i < data_streams_size; ++i) {
        
        // will return null if stream with same name is found in the list
        if (data_streams[i].is_active && !strcmp(data_streams[i].stream_name, stream_name))
        {
            return 1; // TODO: log that stream already exists
        }
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

    new_data_stream->on_ready = on_ready;
    new_data_stream->is_active = true;

    return 0;
}

/**
 * Invokes main action for each stream once
 */
void update_stream()
{    
    if (!is_initialized || !data_streams)
        return;

    // Invoke function for every new stream
    for (int i = 0; i < data_streams_size; ++i) {
        if (data_streams[i].is_active && data_streams[i].on_ready != NULL){

            if(_open_file(&data_streams[i])){
                // TODO: log failed to open the file
                continue;
            }

            data_streams[i].on_ready(&data_streams[i]);

            _close_file(&data_streams[i]);

            _create_flag(&data_streams[i]);
        }
    }

}


static int _open_file(Data_Stream * stream){
    stream->file_ptr = fopen(stream->data_file_path, "w");

    if(stream->file_ptr == NULL)
        return 1; // TODO log error
    return 0;
}

static int _close_file(Data_Stream * stream){
    fclose(stream->file_ptr);
    stream->file_ptr = NULL;
}

static int _create_flag(Data_Stream * stream){
    FILE * temp = fopen(stream->flag_file_path, "w");
    
    if(temp == NULL)
        return 1; // TODO log error

    fclose(temp);
    return 0;
}