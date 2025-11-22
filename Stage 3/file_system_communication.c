/*******************************************************************************
 * Title                 :   File System Communication framework
 * Filename              :   file_system_communication.c
 * Author                :   Dominic
 * Origin Date           :   14/11/2025
 * Version               :   0.0.1
 * Notes                 :   This file creates simple framework that abstracts communication via file system
 *                           between programs to simple API like calls.
 *                           It utilizes callbacks of subscribed functions.
 *                           Single threaded implementation only.
 * TODO:
 * Known issues          :   -Need to check that the codding styles matches the recommendation based on C Style Guidelines
 *                           -Make sure this version of the framework implements all the requested features
 *                           -Test for bugs
 *                           -Need to implement data stream removal function
 *                           -Create data stream should return pointer to created stream for easier management, except there might
 *                           be issue with relocation and use after free, since we are using dynamic array that can be reallocated.
 *                           Consider changing to linked list or fixed max number of streams, to fix it. (or ignore since its already over engineered)
 *******************************************************************************/

#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "file_system_communication.h"

static bool logging_enabled = false;
static Data_Stream *head_data_stream = NULL;

static void _send_line(Data_Stream *context, const char *fmt, ...);
static char *_read_line(Data_Stream *context, char *line_buffer, int max_count);
static Data_Stream * _add_data_stream(void);
static void _populate_data_stream_with_defaults(Data_Stream *stream);
static bool _is_stream_name_valid(const char *stream_name, enum Stream_type stream_type);
static void _populate_stream_data(Data_Stream *stream, const char *stream_name, enum Stream_type stream_type, void (*on_ready)(Data_Stream *));
static void _handle_write_protocol(Data_Stream *stream);
static void _handle_read_protocol(Data_Stream *stream);
static bool _is_data_ready(Data_Stream *stream);
static bool _was_data_read(Data_Stream *stream);
static int _open_data_read(Data_Stream *stream);
static int _open_data_write(Data_Stream *stream);
static void _close_data(Data_Stream *stream);
static int _create_flag(Data_Stream *stream);
static void _remove_flag(Data_Stream *stream);
static int _create_ack(Data_Stream *stream);
static void _remove_ack(Data_Stream *stream);
static int _create_file(const char *file_path);
static bool _file_exists(const char *file_path);
void _log_informative(const char *fmt, ...);
void _log_error(const char *fmt, ...);

/**
 * Enables or disables logging within the File System Communication framework
 * \param enabled true to enable logging, false to disable
 */
void set_file_system_com_framework_logging(bool enabled)
{
    logging_enabled = enabled;
}

/**
 * Will safely close all data streams and return memory
 * \return 0 on success
 */
int close_data_streams()
{
    // free all linked list nodes
    Data_Stream *current = head_data_stream;
    while (current != NULL) {
        Data_Stream *next = current->next;
        free(current);
        current = next;
    }

    head_data_stream = NULL;
    return 0;
}

/**
 * Creates new data stream with specified name
 * and will invoke on_ready every time new frame can be sent
 * \param stream_name max size 80, determines names of the files used in the protocol
 * \param stream_type READ_ONLY_STREAM or WRITE_ONLY_STREAM wether you want to provide or receive data
 * \param on_ready fuction that will be called everytime data is ready to be written to or read from
 * \return non zero if it fails
 */
int create_new_data_stream(const char *stream_name, enum Stream_type stream_type, void (*on_ready)(Data_Stream *))
{
    if (!_is_stream_name_valid(stream_name, stream_type))
    {
        return 1;
    }

    Data_Stream *new_data_stream = _add_data_stream();
    if (!new_data_stream)
    {
        _log_error("ERROR: Failed to create new data stream\n");
        return 1; // TODO: log that stream failed to initialized
    }

    _populate_stream_data(new_data_stream, stream_name, stream_type, on_ready);
    
    _log_informative("INFO: Created new data stream with name %s\n", stream_name);
    return 0;
}

/**
 * Calls the appropriate protocol for each data stream if they are active once
 * Place in a loop to call continuously
 */
void update_streams()
{
    if (!head_data_stream)
    {
        _log_error("ERROR: Data streams not initialized or null\n");
        return;
    }

    _log_informative("INFO: Calling each data stream\n");

    // linked list version
    Data_Stream *current = head_data_stream;
    while (current != NULL) {
        // Skip inactive or un configured streams
        if (current->is_active && current->on_ready != NULL) {
            if (current->stream_type == WRITE_ONLY_STREAM) {
                _handle_write_protocol(current);
            } else if (current->stream_type == READ_ONLY_STREAM) {
                _handle_read_protocol(current);
            }
        }
        current = current->next;
    }
}

/**
 * Function called by framework users to write data to file system. Behaves same as fprintf.
 * \param context contains all the function calls and provides necessary context for the function to be executed on the right files
 * \param fmt format string
 * \param ... parameters specified in format string
 */
static void _send_line(Data_Stream *context, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    vfprintf(context->data_file_ptr, fmt, args);

    va_end(args);
}

/**
 * Function called by framework users to read data from file system. Behaves same as fgets. writes data to buffer
 * until buffer size is reached or until EOF is reached
 * \param context contains all the function calls and provides necessary context for the function to be executed on the right files
 * \param line_buffer string to which data will be written
 * \param max_count size of the buffer
 * \return returns null of EOF was reached
 */
static char *_read_line(Data_Stream *context, char *line_buffer, int max_count)
{
    return fgets(line_buffer, max_count, context->data_file_ptr);
}

/**
 * relallocates more memory for more streams, and sets default values
 */
static Data_Stream * _add_data_stream(void)
{
    Data_Stream *tmp = malloc(sizeof(Data_Stream));
    if (!tmp)
    {
        _log_error("ERROR: Failed to reallocate memory for data streams\n");
        return NULL;
    }

    if (head_data_stream == NULL)
    {
        head_data_stream = tmp;
    }
    else
    {
        Data_Stream *current = head_data_stream;
        while (current->next != NULL)
        {
            current = current->next;
        }
        current->next = tmp;
    }
    _populate_data_stream_with_defaults(tmp);

    return tmp;
}

/**
 * Sets provided data stream with safe known default values
 */
static void _populate_data_stream_with_defaults(Data_Stream *stream)
{
    stream->next = NULL;
    stream->is_active = false;
    stream->is_first_write = true;
    stream->stream_type = READ_ONLY_STREAM;
    stream->on_ready = NULL;
    stream->data_file_ptr = NULL;
    stream->stream_name[0] = '\0';
    stream->flag_file_path[0] = '\0';
    stream->data_file_path[0] = '\0';
    stream->ack_file_path[0] = '\0';
    stream->send_line = _send_line;
    stream->read_line = _read_line;
}



/**
 * Makes sure the name is not over MAX_NAME_LENGTH
 * and that there is no other data stream of the same stream type with same name
 * \return true if name is valid otherwise false
 */
static bool _is_stream_name_valid(const char *stream_name, enum Stream_type stream_type)
{
    // makes sure we don't overflow on the name
    if (strlen(stream_name) >= MAX_NAME_LENGTH)
    {
        _log_error("ERROR: Stream name exceeds maximum length\n");
        return false;
    }

    Data_Stream *current = head_data_stream;
    while (current != NULL)
    {
        // will return null if stream with same name is found in the list
        if (current->is_active && current->stream_type == stream_type && !strcmp(current->stream_name, stream_name))
        {
            _log_error("ERROR: Stream with the same name and type already exists\n");
            return false; // TODO: log that stream already exists
        }
        current = current->next;
    }

    return true;
}

/**
 * populate given data stream with provided data and activates it
 */
static void _populate_stream_data(Data_Stream *stream, const char *stream_name, enum Stream_type stream_type, void (*on_ready)(Data_Stream *))
{
    // Assign the stream name, in a safe way to prevent buffer overflow
    snprintf(stream->stream_name, sizeof(stream->stream_name), "%s", stream_name);

    // generating the dat file name
    snprintf(stream->data_file_path, sizeof(stream->data_file_path), "%s%s", stream_name, DATA_FILE_EXTENSION);

    // generating the flag file name
    snprintf(stream->flag_file_path, sizeof(stream->flag_file_path), "%s%s", stream_name, FLAG_FILE_EXTENSION);

    // generating the ack file name
    snprintf(stream->ack_file_path, sizeof(stream->ack_file_path), "%s%s", stream_name, ACK_FILE_EXTENSION);

    stream->on_ready = on_ready;
    stream->stream_type = stream_type;
    stream->is_active = true;
}

/**
 * Handles reading of data by:
 * - checking for ack / unless its first write
 * - writing the data,
 * - deleting the ack,
 * - sending flag,
 * \param stream contains context necessary to execute the protocol
 */
static void _handle_write_protocol(Data_Stream *stream)
{

    if (!_was_data_read(stream) && !stream->is_first_write)
        return;

    if (_open_data_write(stream))
    {
        _log_error("ERROR: Failed to open data file %s for writing\n", stream->data_file_path);
        return;
    }

    stream->is_first_write = false;

    _log_informative("INFO: Data file %s opened for writing\n", stream->data_file_path);
    // event calling subscribed function
    stream->on_ready(stream);

    _close_data(stream);
    _remove_ack(stream);
    _create_flag(stream);
}

/**
 * Handles reading of data by:
 * - checking for flag
 * - reading the data,
 * - deleting the flag,
 * - sending ack,
 * \param stream contains context necessary to execute the protocol
 */
static void _handle_read_protocol(Data_Stream *stream)
{
    // Skip if flag data is not pressent
    if (!_is_data_ready(stream))
        return;

    if (_open_data_read(stream))
    {
        _log_error("ERROR: Failed to open data file %s for reading\n", stream->data_file_path);
        return;
    }

    _log_informative("INFO: Data file %s opened for reading\n", stream->data_file_path);
    // event calling subscribed function
    stream->on_ready(stream);

    // Properly close handle closing and rasing flags
    _close_data(stream);
    _remove_flag(stream);
    _create_ack(stream);
}

/*
 * Checks for existence of filename.flag file
 * \param stream contains name of the flag file
 * \return true if the filename.flag exists, false otherwise.
 */
static bool _is_data_ready(Data_Stream *stream)
{
    return _file_exists(stream->flag_file_path);
}

/*
 * Checks for existence of filename.ack file
 * \param stream contains name of the ack file
 * \return true if the filename.ack exists, false otherwise.
 */
static bool _was_data_read(Data_Stream *stream)
{
    return _file_exists(stream->ack_file_path);
}

/**
 * Opens the main data file in a write mode and stores the file pointer in stream's data_file_ptr
 * \return 0 if all goes well
 */
static int _open_data_write(Data_Stream *stream)
{
    stream->data_file_ptr = fopen(stream->data_file_path, "w");

    if (stream->data_file_ptr == NULL)
        return 1;
    return 0;
}

/**
 * Opens the main data file in a read mode and stores the file pointer in stream's data_file_ptr
 * \return 0 if all goes well
 */
static int _open_data_read(Data_Stream *stream)
{
    stream->data_file_ptr = fopen(stream->data_file_path, "r");

    if (stream->data_file_ptr == NULL)
        return 1;
    return 0;
}

/**
 * Closes the main data file at sets it to NULL to prevent use after free
 */
static void _close_data(Data_Stream *stream)
{
    if (stream->data_file_ptr != NULL) {
        fclose(stream->data_file_ptr);
        stream->data_file_ptr = NULL;
    }
}

/**
 * Creates flag file
 * \return 0 if all goes well
 */
static int _create_flag(Data_Stream *stream)
{
    return _create_file(stream->flag_file_path);
}

/**
 * Removes the flag file
 */
static void _remove_flag(Data_Stream *stream)
{
    remove(stream->flag_file_path);
}

/**
 * Creates ack file
 * \return 0 if all goes well
 */
static int _create_ack(Data_Stream *stream)
{
    return _create_file(stream->ack_file_path);
}

/**
 * Removes ack file
 */
static void _remove_ack(Data_Stream *stream)
{
    remove(stream->ack_file_path);
}

/**
 * Creates file and closes it
 * \return 0 if all goes well
 */
static int _create_file(const char *file_path)
{
    FILE *temp = fopen(file_path, "w");

    if (temp == NULL)
    {
        _log_error("ERROR: Failed to create file %s\n", file_path);
        return 1;
    }

    fclose(temp);
    return 0;
}

/*
 * _file_exists function Checks if a file exists.
 * \param filename is the name of the file to check.
 * \return true if the file exists, false otherwise.
 */
static bool _file_exists(const char *file_path)
{
    FILE *file = fopen(file_path, "r");
    if (file)
    {
        fclose(file);
        return true;
    }
    return false;
}

/**
 * Informative logging function, ignores if logging is disabled
 */
void _log_informative(const char *fmt, ...)
{
    if (logging_enabled)
    {
        va_list args;
        va_start(args, fmt);

        vfprintf(stdout, fmt, args);

        va_end(args);
    }
}

/**
 * Error logging function
 */
void _log_error(const char *fmt, ...)
{

    va_list args;
    va_start(args, fmt);

    vfprintf(stderr, fmt, args);

    va_end(args);
}