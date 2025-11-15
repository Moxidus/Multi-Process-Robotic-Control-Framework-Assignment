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

//cross-platform sleep
#ifdef _WIN32
#include <windows.h>
#define sleep_ms(ms) Sleep(ms)
#else
#include <unistd.h>
#define sleep_ms(ms) usleep(ms * 1000)
#endif

//flag file names for communication
#define DATA_FILE "lidar_data.txt"
#define READY_FLAG "data_ready.flag"

//polling interval in ms
#define POLL_INTERVAL_MS 100

/*
 * file_exists function checks if a file exists.
 * filename is the name of the file to check.
 * file_exists return 1 if the file exists, 0 otherwise.
 */
int file_exists(const char *filename)
{
    FILE *file = fopen(filename, "r");
    if (file)
    {
        fclose(file);
        return 1;
    }
    return 0;
}

/*
 * create_file Creates an empty file.
 * filename is the name of the file to create.
 */
void create_file(const char *filename)
{
    FILE *file = fopen(filename, "w");
    if (file)
    {
        fclose(file);
    }
    else
    {
        fprintf(stderr, "Error creating file: %s\n", filename);
    }
}

int main()
{
    int data_counter = 0;
    //seeding RNG
    srand(time(NULL));

    printf("Process A (sensor_lidar) started.\n");
    printf("This process writes to %s and signals with %s.\n", DATA_FILE, READY_FLAG);

    //ensuring a clean start by removing any stale flags from a previous run
    remove(READY_FLAG);

    while (1)
    {
        // Wait for ACK by ready flag being deleted
        if(file_exists(READY_FLAG)){
            continue;
        }

        //--- write new LIDAR data ---
        data_counter++;

		//creating a custom hash/verifier code for each data packet
        int verifier_code = data_counter * 13 % 1000;

        printf("Writing data packet %d (Verify Code: %d) to %s...\n", data_counter, verifier_code, DATA_FILE);

        FILE *file = fopen(DATA_FILE, "w");
        if (file == NULL)
        {
            fprintf(stderr, "Error: Could not open %s for writing.\n", DATA_FILE);
            return 1; //exit on error
        }
        //generate some LIDAR data
        //angle_min in range -2.0 to -1.0
        double angle_min = -2.0 + (double)rand() / RAND_MAX * 1.0;
        //angle_max in range 1.0 to 2.0
        double angle_max = 1.0 + (double)rand() / RAND_MAX * 1.0;
        //range readings between 5.0 and 15.0
        double range_0 = 5.0 + (double)rand() / RAND_MAX * 10.0;
        double range_1 = 5.0 + (double)rand() / RAND_MAX * 10.0;
        double range_2 = 5.0 + (double)rand() / RAND_MAX * 10.0;

		fprintf(file, "packet_id: %d\n", data_counter);
        fprintf(file, "verifier_code: %d\n", verifier_code);
        fprintf(file, "angle_min: %.2f\n", angle_min);
        fprintf(file, "angle_max: %.2f\n", angle_max);
        fprintf(file, "range_0: %.2f\n", range_0);
        fprintf(file, "range_1: %.2f\n", range_1);
        fprintf(file, "range_2: %.2f\n", range_2);
        fclose(file);

        //signal ready flag (data_ready.flag )
        printf("Signalling new data is ready (creating %s).\n\n", READY_FLAG);
        create_file(READY_FLAG);

        //set sensor refresh rate (here 1 Hz - 1 sample per second)
        sleep_ms(1000);
    }

    return 0;
}