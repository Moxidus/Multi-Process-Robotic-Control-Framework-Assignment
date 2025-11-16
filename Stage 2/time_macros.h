/*******************************************************************************
* Title                 :   Time Macros
* Filename              :   time_macros.h
* Author                :   Dominic, Karl
* Origin Date           :   16/11/2025
* Version               :   0.0.1
* Notes                 :   none
*******************************************************************************/
#ifndef TIME_MACROS_H
#define TIME_MACROS_H

#include <time.h>

//cross-platform sleep
#ifdef _WIN32
#include <windows.h>
#define sleep_ms(ms) Sleep(ms)
#else
#include <unistd.h>
#define sleep_ms(ms) usleep(ms * 1000)
#endif

#endif