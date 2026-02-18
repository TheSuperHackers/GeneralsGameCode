/* macOS shim: direct.h — MSVC directory operations mapped to POSIX */
#pragma once
#include <sys/stat.h>
#include <unistd.h>

#define _getcwd getcwd
#define _chdir chdir
#define _mkdir(path) mkdir(path, 0755)
#define _stat stat
#define _fstat fstat
