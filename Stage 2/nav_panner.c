/*******************************************************************************
* Title                 :   Navigation Planner
* Filename              :   nav_panner.c
* Author                :   Dominic, Karl
* Origin Date           :   14/11/2025
* Version               :   0.0.1
* Notes                 :   This is over engineered implementation of the stage 2 solution for Navigation planner.
*                           All the file manipulation and data management is abstracted away and handled by file_system_communication.c
*                           It gives us ability to send data, manage multiple sending and reading streams
*                           and prevents race conditions by using flags and acks.
*******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include "file_system_communication.h"
#include "time_macros.h"

#define LIDAR_STREAM_NAME "lidar_data"

//polling interval (ms)
#define POLL_INTERVAL_MS 1000

void main_loop();
void receiving_data(Data_Stream *context);

int main()
{
    //seeding RNG
    srand(time(NULL));
    
    // Enable logging for File System Communication framework
    set_file_system_com_framework_logging(true);
    
    // We create the sending data stream with name sensor_lidar, and pass our handle function to the event handler
    if(create_new_data_stream(LIDAR_STREAM_NAME, READ_ONLY_STREAM, receiving_data)){
        fprintf(stderr, "We failed to create new stream!\n");
        return 1;
    }

    fprintf(stdout, "Process B (nav_planner) started.\n");
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
