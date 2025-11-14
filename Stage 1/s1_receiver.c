/*
 * file: receiver.c
 * Stage 1: Process B (nav_planner)
 *
 * This acts as the data consumer (Navigation Planner).
 * 1. Waits for the sender (Process A) to signal data is ready (by checking for data_ready.flag).
 * 2. Deletes the data_ready.flag (consumes the signal).
 * 3. Reads the data from lidar_data.txt.
 * 4. Repeats in a loop.
 *
 * This file-based owe way no ack IPC mechanism.
 */

#include <stdio.h>
#include <stdlib.h>

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

//polling interval (ms)
#define POLL_INTERVAL_MS 100

/*
 * file_exists function Checks if a file exists.
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

int main() {
    char line_buffer[256]; //a buffer memory to read lines from the data file

    printf("Process B (nav_planner) started.\n");
    printf("This process reads from %s and waits for %s.\n\n", DATA_FILE, READY_FLAG);

    while (1)
    {
        //---wait for sender to be ready (till the data_ready.flag exists)---
        printf("Waiting for new data (checking for %s)...\n", READY_FLAG);
        while (!file_exists(READY_FLAG))
        {
            sleep_ms(POLL_INTERVAL_MS);
        }

        //---consume the signal (and delete the flag file) ---
        if (remove(READY_FLAG) != 0)
        {
            fprintf(stderr, "Warning: Could not remove %s. Retrying...\n", READY_FLAG);
            continue; //retry the loop
        }
        printf("New data is ready. Deleting %s.\n", READY_FLAG);

        //---read the data ---
        printf("Reading data from %s...\n", DATA_FILE);
        FILE *file = fopen(DATA_FILE, "r");
        if (file == NULL)
        {
            fprintf(stderr, "Error: Could not open %s for reading.\n", DATA_FILE);
            //wait for next loop without signaling ACK
            sleep_ms(1000);
            continue;
        }

        //read the file line by line and print it
        printf("--- [DATA START] ---\n");
        while (fgets(line_buffer, sizeof(line_buffer), file) != NULL)
        {
            printf("  %s", line_buffer); //print the line (includes newline)
        }
        printf("--- [DATA END] ---\n");
        fclose(file);

        //adding a small amount of processing time
        sleep_ms(200);

    }

    return 0;
}