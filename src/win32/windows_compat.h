#ifndef WINDOWS_COMPAT_H
#define WINDOWS_COMPAT_H

#ifdef _WIN32

// Windows compatibility header for QTerminal
// This header provides cross-platform compatibility for Windows

#include <windows.h>
#include <io.h>
#include <direct.h>
#include <process.h>
#include <errno.h>
#include <cstring>

// Map Unix functions to Windows equivalents
#define chdir _chdir
#define getcwd _getcwd
#define getpid _getpid
#define access _access
#define unlink _unlink
#define fileno _fileno
#define isatty _isatty
#define close _close
#define write _write
#define read _read
#define open _open
#define lseek _lseek
#define stat _stat
#define fstat _fstat

// Define missing constants
#ifndef F_OK
#define F_OK 0
#endif

#ifndef R_OK
#define R_OK 4
#endif

#ifndef W_OK
#define W_OK 2
#endif

#ifndef X_OK
#define X_OK 1
#endif

// Define missing file modes
#ifndef S_IRUSR
#define S_IRUSR _S_IREAD
#endif

#ifndef S_IWUSR
#define S_IWUSR _S_IWRITE
#endif

#ifndef S_IXUSR
#define S_IXUSR _S_IEXEC
#endif

#ifndef S_IRGRP
#define S_IRGRP 0
#endif

#ifndef S_IWGRP
#define S_IWGRP 0
#endif

#ifndef S_IXGRP
#define S_IXGRP 0
#endif

#ifndef S_IROTH
#define S_IROTH 0
#endif

#ifndef S_IWOTH
#define S_IWOTH 0
#endif

#ifndef S_IXOTH
#define S_IXOTH 0
#endif

// Define missing types
typedef int pid_t;
typedef int uid_t;
typedef int gid_t;
typedef int mode_t;
typedef int ssize_t;

// Error handling
inline const char* strerror_safe(int errnum) {
    static char buffer[256];
    strerror_s(buffer, sizeof(buffer), errnum);
    return buffer;
}

// Define strerror macro for compatibility
#ifndef strerror
#define strerror(x) strerror_safe(x)
#endif

// Sleep function compatibility
inline void sleep(unsigned int seconds) {
    Sleep(seconds * 1000);
}

inline void usleep(unsigned int microseconds) {
    Sleep(microseconds / 1000);
}

#endif // _WIN32

#endif // WINDOWS_COMPAT_H
