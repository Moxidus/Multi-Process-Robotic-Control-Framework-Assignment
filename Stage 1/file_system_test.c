#include <stdio.h>
#include "file_system_communication.h"


void on_ready(){
    printf("Sending data\n");
}


int main(){

    printf("We have compiled!\n");

    if(init_data_streams()){
        printf("We failed to initialize!\n");
        return 1;
    }
    
    
    if(create_new_data_stream("Rover_Send", on_ready)){
        printf("We failed to create new stream!\n");
        return 1;
    }
    
    update_stream();
    update_stream();
    update_stream();
    update_stream();


    return 0;
}

