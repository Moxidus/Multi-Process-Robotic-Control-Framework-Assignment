# Author: Dominic
# This was created for windows but it should be easy to change the commands to work on LINUX
# or just just write the gcc commands by hand, its not a big project

# Compiler and flags
CC      := gcc
CFLAGS  := -Wall -Wextra -Wpedantic -std=c99 -g

# Build directory
BUILD_DIR := build
# object directory
OBJ_DIR := $(BUILD_DIR)\obj
# exe directory
BIN_DIR := $(BUILD_DIR)\\bin


# Source files
SRCS := file_system_communication.c nav_panner.c sensor_lidar.c

# Objects stored in build/obj
OBJS := $(SRCS:%.c=$(OBJ_DIR)/%.o)

# Executables are stored in build/bin
NAV_PLANNER := $(BIN_DIR)/nav_panner.exe
SENSOR_LIDAR   := $(BIN_DIR)/sensor_lidar.exe

.PHONY: all clean dirs nav_panner sensor_lidar

all: dirs $(NAV_PLANNER) $(SENSOR_LIDAR)

# Build only nav_panner
nav_panner: dirs $(NAV_PLANNER)
	@echo Built $(NAV_PLANNER)

# Build only sensor_lidar
sensor_lidar: dirs $(SENSOR_LIDAR)
	@echo Built $(SENSOR_LIDAR)

dirs:
	if not exist $(OBJ_DIR) mkdir $(OBJ_DIR)
	if not exist $(BIN_DIR) mkdir $(BIN_DIR)

# Build nav_panner
$(NAV_PLANNER): $(OBJ_DIR)/file_system_communication.o $(OBJ_DIR)/nav_panner.o
	$(CC) $(CFLAGS) -o $@ $^

# Build sensor_lidar
$(SENSOR_LIDAR): $(OBJ_DIR)/file_system_communication.o $(OBJ_DIR)/sensor_lidar.o
	$(CC) $(CFLAGS) -o $@ $^

# Generic object file rule
$(OBJ_DIR)/%.o: %.c file_system_communication.h | dirs
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	if exist $(BUILD_DIR) rmdir /s /q $(BUILD_DIR)
