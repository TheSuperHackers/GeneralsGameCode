#pragma once
#include <pthread.h>
#include <unistd.h>

inline int GetCurrentThreadId()
{
  return pthread_self();
}

inline void Sleep(int ms)
{
  usleep(ms * 1000);
}