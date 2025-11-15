/*
 * file: sender.c
 * Stage 1: Process A (sensor_lidar)
 *
 * This acts as the data producer (LIDAR sensor).
 * 1. Writes new simulated data to lidar_data.txt.
 * 2. Creates data_ready.flag to signal that new data is available.
 * 3. Repeats in a loop.
 *
 * Not a robust Implementation, as it does not wait for receiver to send read
 * acknowledgment, leading to a race condition.
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

    printf("Process A (sensor_lidar) started.\n");
    // printf("This process writes to %s and signals with %s.\n", DATA_FILE, READY_FLAG);

    if(init_data_streams()){
        printf("We failed to initialize!\n");
        return 1;
    }
    
    if(create_new_data_stream(LIDAR_STREAM_NAME, WRITE, sending_data)){
        printf("We failed to create new stream!\n");
        return 1;
    }

    main_loop();

    return 0;
}

/**
 * Main loop that loops through the main logic of the code
 */
void main_loop(){
    while (1)
    {
        update_stream();

        //set sensor refresh rate (here 1 Hz - 1 sample per second)
        sleep_ms(POLL_INTERVAL_MS);
    }
}

void sending_data(Data_Stream *context)
{
    data_counter++;

    // creating a custom hash/verifier code for each data packet
    int verifier_code = data_counter * 13 % 1000;

    printf("Writing data packet %d (Verify Code: %d) to %s...\n", data_counter, verifier_code, context->data_file_path);

    // generate some LIDAR data
    // angle_min in range -2.0 to -1.0
    double angle_min = -2.0 + (double)rand() / RAND_MAX * 1.0;
    // angle_max in range 1.0 to 2.0
    double angle_max = 1.0 + (double)rand() / RAND_MAX * 1.0;
    // range readings between 5.0 and 15.0
    double range_0 = 5.0 + (double)rand() / RAND_MAX * 10.0;
    double range_1 = 5.0 + (double)rand() / RAND_MAX * 10.0;
    double range_2 = 5.0 + (double)rand() / RAND_MAX * 10.0;

    char buffer[254];

    snprintf(buffer, sizeof(buffer), "packet_id: %d\n", data_counter);
    context->send_line(context, buffer);

    snprintf(buffer, sizeof(buffer), "verifier_code: %d\n", verifier_code);
    context->send_line(context, buffer);

    snprintf(buffer, sizeof(buffer), "angle_min: %.2f\n", angle_min);
    context->send_line(context, buffer);

    snprintf(buffer, sizeof(buffer), "angle_max: %.2f\n", angle_max);
    context->send_line(context, buffer);

    snprintf(buffer, sizeof(buffer), "range_0: %.2f\n", range_0);
    context->send_line(context, buffer);

    snprintf(buffer, sizeof(buffer), "range_1: %.2f\n", range_1);
    context->send_line(context, buffer);

    snprintf(buffer, sizeof(buffer), "range_2: %.2f\n", range_2);
    context->send_line(context, buffer);
}
