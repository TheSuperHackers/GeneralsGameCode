/* macOS shim: process.h — maps MSVC process API to POSIX */
#pragma once
#include <pthread.h>
#include <unistd.h>

#define _getpid getpid
