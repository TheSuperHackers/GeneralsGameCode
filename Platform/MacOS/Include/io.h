/* macOS shim: io.h — MSVC low-level I/O mapped to POSIX */
#pragma once
#include <fcntl.h>
#include <unistd.h>

#define _open open
#define _close close
#define _read read
#define _write write
#define _lseek lseek
#define _access access
#define _O_RDONLY O_RDONLY
#define _O_WRONLY O_WRONLY
#define _O_RDWR O_RDWR
#define _O_CREAT O_CREAT
#define _O_TRUNC O_TRUNC
#define _O_BINARY 0
