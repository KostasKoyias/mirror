#ifndef utils_H_
#define utils_H_
#include "define.h"
#include "sync.h"

/* utility methods */
void usage_error();
void perror_exit(char*);
void error_exit(const char*,...);
int error_return(int, const char*, ...);
int exclusive_print(int, const char*, ...);
int sync_error(int, int, const char*, ...);
int alarm_read(int, void*, size_t, unsigned int);

/* sync info list member methods */
int assign(void*, const void*);
void *compare(void*, const void*);
int print(void*);

#endif