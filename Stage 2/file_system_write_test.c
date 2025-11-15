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
    static int data_counter = 0;
    data_counter++;
    int verifier_code = data_counter * 13 % 1000;

    char buffer[100];
    snprintf(buffer, sizeof(buffer), "packet_id: %d\n", data_counter);

    context->send_line(context, buffer);

    printf("Sending data\n");
}


int main(){

    printf("We have compiled!\n");

    if(init_data_streams()){
        printf("We failed to initialize!\n");
        return 1;
    }
    
    
    if(create_new_data_stream("lidar_data", WRITE, on_ready)){
        printf("We failed to create new stream!\n");
        return 1;
    }
    
    // update_stream();
    loop();

    return 0;
}

void loop(){
    while (1)
    {

        update_stream();

        sleep_ms(1000);
    }
}

