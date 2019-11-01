#ifndef define_H_
#define define_H_

#include <stdio.h>      
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>      //low-level I/O syscalls
#include <stdarg.h>
#include <stdint.h>
#include <sys/file.h>   //flock
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/wait.h>
#include <time.h>
#include <semaphore.h>
#include <errno.h>
#define MIN(a,b) ((a) < (b)) ? (a) : (b)
#define FILE_PERMS 0644
#define DIR_PERMS 0744  
#define MAX_BUFFER 2048      // buffers of this size, usually store absolute paths, so they need to be big
#define MAX_ID 8             // ids have no more than 7 digits
#define MAX_DIR 30           // maximum dir name set to 30 characters
#define MAX_FILE 30          // maximum file name set to 30 characters
#define MAX_WAIT_READ 15     // block no more than MAX_WAIT_READ seconds to either open or read on a named-pipe 
#define MAX_WAIT_WRITE 10    // block no more than MAX_WAIT_WRITE seconds to write on a named-pipe
#define MAX_PID 8            // process ids have no more than 7 digits in most systems
#define CYCLE 10             // monitor cycle set to CYCLE seconds

void handler(int, siginfo_t*, void*);
void free_rsrc(int);

struct SyncInfo{
    pid_t pid;
    int id;
    uint8_t code;
    uint8_t tries;
};

#endif