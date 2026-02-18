#pragma once

#ifdef __cplusplus
extern "C" {
#endif

void* MacOS_CreateWindow(int width, int height, const char *title);
bool MacOS_ShouldQuit();
void MacOS_PumpEvents();
int MacOS_Main(int argc, char *argv[]);

#ifdef __cplusplus
}
#endif
