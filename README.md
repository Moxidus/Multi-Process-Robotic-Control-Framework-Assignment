# Multi Process Robotic Control Framework Assignment
This assignment implements most of the functionality in file_system_communication for the stage 2 and 3. And implements all Mutex logging features in the mutex_logging.
These frameworks are then imported into the nav_planner sensor_lidar and motor_ctrl.
building is done using gcc and make.

## Requirements
- make
- gcc c99 compiler

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