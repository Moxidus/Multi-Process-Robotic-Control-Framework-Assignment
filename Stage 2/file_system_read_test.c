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
    // context->send_line(context, "Hello world\n");
    char line_buffer[256];
    while (context->read_line(context, line_buffer, sizeof(line_buffer)))
    {
        printf("%s", line_buffer);
    }
}


int main(){

    printf("We have compiled!\n");

    if(init_data_streams()){
        printf("We failed to initialize!\n");
        return 1;
    }
    
    
    if(create_new_data_stream("lidar_data", READ, on_ready)){
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


