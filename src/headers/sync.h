#ifndef sync_H_
#define sync_H_
#include <semaphore.h>
#include "define.h"
#include "cmd.h"
#include "../../gen-list/list.h"

struct cmd;

void alarm_handler(int);
int send_folder(const char*, DIR*, int, const char*);
int send_content(int);
int recv_folder(const char*, int, const char*);
int recv_content(int);
int sync_with(int);

#endif