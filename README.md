# Multi Process Robotic Control Framework Assignment
This assignment implements most of the functionality in file_system_communication for the stage 2 and 3. And implements all Mutex logging features in the mutex_logging.
These frameworks are then imported into the nav_planner sensor_lidar and motor_ctrl.
building is done using gcc and make.

## Requirements
- make
- gcc c99 compiler
- add these binaries to the environment variables

## Building the project
1. navigate to any stage of the project:
```
cd stage 2
```
2. build the project using the make command (make sure to have 'make' and 'gcc' installed and added to your environment variables)
```
make all
```
3. navigate to 'build/bin' to find the executables of all programs


## Examples
example on how to use File system communication framework
```
int main()
{
    // Initializes the File System Communication framework
    init_data_streams()
    
    // We create the write data stream
    create_new_data_stream("unique_name", WRITE_ONLY_STREAM, sending_data)

    while (1)
    {
        // Function provided by File System Communication framework that automatically invokes events when data is ready
        update_streams();

        sleep_ms(1000);
    }

    return 0;
}

// this function is going to be called by 
void sending_data(Data_Stream *context)
{
    int number;
    context->send_line(context, "data: %d\n", number);
    context->send_line(context, "end of data");
}


```