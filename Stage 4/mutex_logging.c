#include "mutex_logging.h"
#include <stdio.h>
#include <stdbool.h>

//cross-platform sleep
#ifdef _WIN32
#include <windows.h>
#define sleep_ms(ms) Sleep(ms)
#else
#include <unistd.h>
#define sleep_ms(ms) usleep(ms * 1000)
#endif


// Check if log.lock exists
bool lock_exists(void) {
    // try to open the file
    FILE *f = fopen("log.lock", "r");
    // if file doesnt exist
    if (f == NULL) 
        return false;
    fclose(f);
    return true;
}

// writes logs to file
void record_log(char message[]){

    // Wait while lock exists if true allow message to be written
    while (lock_exists()) { //  
        sleep_ms(100);  // 100ms
    }

    // Create lock file 
    FILE *lock_file = fopen("log.lock", "w");
    if (lock_file == NULL) {
        return;
    }
    fclose(lock_file);

    // Open log file
    FILE *log_file = fopen("system_log.txt", "a"); 
    if (log_file == NULL) {
        remove("log.lock");
        return ;
    }

    //writes the message to system log file
    fprintf(log_file, "\n%s", message);
    //close the system file
    fclose(log_file);
    remove("log.lock");

}