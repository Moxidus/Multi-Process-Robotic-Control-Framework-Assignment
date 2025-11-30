/*
 * file: nav_panner.c
 * Stage 2: Two-way two process IPC
 * Created by: Dominic, Karl
 *
 * This acts as the data consumer (Navigation Planner).
 * 1. Waits for the sender (Process A) to signal data is ready (by checking for data_ready.flag).
 * 2. Deletes the data_ready.flag (consumes the signal).
 * 3. Reads the data from lidar_data.txt.
 * 4. Repeats in a loop.
 *
 * This is over engineered implementation of the stage 2 solution for receiver.
 * All the file manipulation and data management is abstracted away and handled by file_system_communication.c
 * It gives us ability to send data, manage multiple sending and reading streams
 * and prevents race conditions by using flags and acs
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "file_system_communication.h"
#include "mutex_logging.h"

//cross-platform sleep
#ifdef _WIN32
#include <windows.h>
#define sleep_ms(ms) Sleep(ms)
#else
#include <unistd.h>
#define sleep_ms(ms) usleep(ms * 1000)
#endif

#define LIDAR_STREAM_NAME "lidar_data"
#define MOTOR_STREAM_NAME "motor_commands"

//polling interval (ms)
#define POLL_INTERVAL_MS 1000

static int data_counter = 0;

void main_loop();
void receiving_data(Data_Stream *context);
void sending_motor_commands(Data_Stream *context);

int main()
{
    //seeding RNG
    srand(time(NULL));
    
    // Enable logging for File System Communication framework
    set_file_system_com_framework_logging(true);

    // We create the sending data stream with name sensor_lidar, and pass our handle function to the event handler
    if(create_new_data_stream(LIDAR_STREAM_NAME, READ_ONLY_STREAM, receiving_data)){
        fprintf(stderr, "We failed to create new stream!\n");
        record_log("[Navigation]: We failed to create new stream!");
        return 1;
    }

    if(create_new_data_stream(MOTOR_STREAM_NAME, WRITE_ONLY_STREAM, sending_motor_commands)){
        fprintf(stderr, "We failed to create new motor command stream!\n");
        record_log( "[Navigation]: We failed to create new motor command stream!");
        return 1;
    }

    fprintf(stdout, "Process B (nav_planner) started.\n");
    record_log("[Navigation]: Process B (nav_planner) started.\n");
    fprintf(stdout, "This process reads from %s using the File System Communication framework\n\n", LIDAR_STREAM_NAME);

    // Calling the main loop, program will hold here until its terminated 
    main_loop();

    return 0;
}

/**
 * Main loop that loops through the main logic of the code
 * with selected interval
 */
void main_loop(){
    while (1)
    {
        // Function provided by File System Communication framework that automatically invokes events when data is ready
        update_streams();

        // Set sensor refresh rate (here 1 Hz - 1 sample per second)
        sleep_ms(POLL_INTERVAL_MS);
    }
}

/**
 * This function is automatically called by the File System Communication framework,
 * when its the first call and when data has been read by the receiver.
 * It generates mock data and sends them using the provided framework
 */
void receiving_data(Data_Stream *context)
{
    char line_buffer[256]; //a buffer memory to read lines from the data file

    //---read the data ---
    fprintf(stdout, "\n\nReading data from %s...\n", context->data_file_path);
    
    printf("--- [DATA START] ---\n");
    while (context->read_line(context, line_buffer, sizeof(line_buffer)) != NULL)
    {
        printf("  %s", line_buffer); // print the line (includes newline)
    }
    printf("--- [DATA END] ---\n");
}


void sending_motor_commands(Data_Stream *context)
{
    data_counter++;

    fprintf(stdout, "Writing data packet %d to %s...\n", data_counter, context->data_file_path);

    // generate some LIDAR data
    // speed_left in range 0.0 to 1.0
    double speed_left = (double)rand() / RAND_MAX * 1.0;
    // speed_right in range 0.0 to 1.0
    double speed_right = (double)rand() / RAND_MAX * 1.0;

    const char * direction = rand() > RAND_MAX / 2 ? "FORWARD" : "BACKWARD";

    // writing the data to the stream
    context->send_line(context, "command_id: %d\n", data_counter);
    context->send_line(context, "speed_left: %.2f\n", speed_left);
    context->send_line(context, "speed_right: %.2f\n", speed_right);
    context->send_line(context, "direction: %s\n", direction);

}