/* macOS shim: d3dx8tex.h — texture helpers, forwarded to d3dx8core.h */
#pragma once
#include "d3dx8core.h"

/* Convenience alias used by some code */
#ifndef D3DXCreateTextureFromFileEx
#define D3DXCreateTextureFromFileEx D3DXCreateTextureFromFileExA
#endif

#ifndef D3DXSaveTextureToFile
#define D3DXSaveTextureToFile D3DXSaveTextureToFileA
#endif

#ifndef D3DXGetImageInfoFromFile
#define D3DXGetImageInfoFromFile D3DXGetImageInfoFromFileA
#endif
