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
SRCS := file_system_communication.c s2_receiver.c s2_sender.c

# Objects stored in build/obj
OBJS := $(SRCS:%.c=$(OBJ_DIR)/%.o)

# Executables are stored in build/bin
RECEIVER := $(BIN_DIR)/s2_receiver.exe
SENDER   := $(BIN_DIR)/s2_sender.exe

.PHONY: all clean dirs

all: dirs $(RECEIVER) $(SENDER)

dirs:
	if not exist $(OBJ_DIR) mkdir $(OBJ_DIR)
	if not exist $(BIN_DIR) mkdir $(BIN_DIR)

# Build receiver
$(RECEIVER): $(OBJ_DIR)/file_system_communication.o $(OBJ_DIR)/s2_receiver.o
	$(CC) $(CFLAGS) -o $@ $^

# Build sender
$(SENDER): $(OBJ_DIR)/file_system_communication.o $(OBJ_DIR)/s2_sender.o
	$(CC) $(CFLAGS) -o $@ $^

# Generic object file rule
$(OBJ_DIR)/%.o: %.c file_system_communication.h | dirs
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	if exist $(BUILD_DIR) rmdir /s /q $(BUILD_DIR)
