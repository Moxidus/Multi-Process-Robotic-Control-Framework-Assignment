/*
 * file: s2_sender.c
 * Stage 2: Two-way two process IPC
 * Created by: Dominic, Karl
 *
 * This acts as the data producer (LIDAR sensor).
 * 1. Waits for lidar_data.ack from the receiver unless its the first invocation of the stream
 * 2. Writes new simulated data to lidar_data.txt.
 * 3. Creates lidar_data.flag to signal that new data is available.
 * 4. Repeats in a loop.
 *
 * This is over engineered implementation of the stage 2 solution.
 * All the file manipulation and data management is abstracted away and handled by file_system_communication.c
 * It gives us ability to send data, manage multiple sending and reading streams
 * and prevents race conditions by using flags and acs
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "file_system_communication.h"

//cross-platform sleep
#ifdef _WIN32
#include <windows.h>
#define sleep_ms(ms) Sleep(ms)
#else
#include <unistd.h>
#define sleep_ms(ms) usleep(ms * 1000)
#endif

#define LIDAR_STREAM_NAME "lidar_data"

//polling interval (ms)
#define POLL_INTERVAL_MS 100

void main_loop();
void sending_data(Data_Stream * context);

int data_counter = 0;

int main()
{
    //seeding RNG
    srand(time(NULL));

    // Initializes the File System Communication Api
    if(init_data_streams()){
        fprintf(stderr, "We failed to initialize!\n");
        return 1;
    }
    
    // We create the sending data stream with name sensor_lidar, and pass our handle function to the event handler
    if(create_new_data_stream(LIDAR_STREAM_NAME, WRITE, sending_data)){
        fprintf(stderr, "We failed to create new stream!\n");
        return 1;
    }

    fprintf(stdout, "Process A (sensor_lidar) started.\n");
    fprintf(stdout, "This process writes to %s using the File System Communication Api\n", LIDAR_STREAM_NAME);

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
        // Function provided by File System Communication Api that automatically invokes events when data is ready
        update_stream();

        // Set sensor refresh rate (here 1 Hz - 1 sample per second)
        sleep_ms(POLL_INTERVAL_MS);
    }
}

/**
 * This function is automatically called by the File System Communication Api,
 * when its the first call and when data has been read by the receiver.
 * It generates mock data and sends them using the provided api
 */
void sending_data(Data_Stream *context)
{
    data_counter++;

    // creating a custom hash/verifier code for each data packet
    int verifier_code = data_counter * 13 % 1000;

    fprintf(stdout, "Writing data packet %d (Verify Code: %d) to %s...\n", data_counter, verifier_code, context->data_file_path);

    // generate some LIDAR data
    // angle_min in range -2.0 to -1.0
    double angle_min = -2.0 + (double)rand() / RAND_MAX * 1.0;
    // angle_max in range 1.0 to 2.0
    double angle_max = 1.0 + (double)rand() / RAND_MAX * 1.0;
    // range readings between 5.0 and 15.0
    double range_0 = 5.0 + (double)rand() / RAND_MAX * 10.0;
    double range_1 = 5.0 + (double)rand() / RAND_MAX * 10.0;
    double range_2 = 5.0 + (double)rand() / RAND_MAX * 10.0;

    // writing the data to the stream
    context->send_line(context, "packet_id: %d\n", data_counter);
    context->send_line(context, "verifier_code: %d\n", verifier_code);
    context->send_line(context, "angle_min: %.2f\n", angle_min);
    context->send_line(context, "angle_max: %.2f\n", angle_max);
    context->send_line(context, "range_0: %.2f\n", range_0);
    context->send_line(context, "range_1: %.2f\n", range_1);
    context->send_line(context, "range_2: %.2f\n", range_2);
}
