/* test is dissabled for now */
/*
#include <stdio.h>
#include <stdlib.h>
#include "file_system_communication.h"

//cross-platform sleep
#ifdef _WIN32
#include <windows.h>
#define sleep_ms(ms) Sleep(ms)
#else
#include <unistd.h>
#define sleep_ms(ms) usleep(ms * 1000)
#endif

//polling interval (ms)
#define POLL_INTERVAL_MS 100

void loop();

void on_ready(Data_Stream * context){

    context->send_line(context, "test data line \n");

    printf("Sending data\n");
}


int main(){

    printf("We have compiled!\n");
    
    if(create_new_data_stream("test_data", WRITE_ONLY_STREAM, on_ready)){
        printf("We failed to create new stream!\n");
        return 1;
    }
    
    // update_streams();
    loop();

    return 0;
}

void loop(){
    while (1)
    {

        update_streams();

        sleep_ms(POLL_INTERVAL_MS);
    }
}

*/