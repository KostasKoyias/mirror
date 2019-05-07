#ifndef _cmd_H
#define _cmd_H

#include "define.h"
#include "utils.h"

struct cmd{
    DIR *common;
    DIR *input;
    DIR *mirror;
    int id;
    int buffer_sz;
    int logfile;
    uint8_t common_i;
    uint8_t input_i;
    uint8_t mirror_i;
};

void cmd_check(int, char**, struct cmd*);

#endif 