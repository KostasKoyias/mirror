/***********************************************************************************************\
 * The following are some short utility functions, making code re-use simpler                   *
 * most of them handle common error cases and code that is widely used through out this project *
\***********************************************************************************************/
#include "utils.h"

// print a usage message and exit with code 1(FAILURE)
void usage_error(){
    fprintf(stdout, "Usage: mirror -n id -c common_dir -i input_dir -m mirror_dir -b buffer_size -l"\
    " log_file [-rm]\ninput_dir shall exist, but mirror_dir should not\nFirst six arguments are mandatory\n");
    exit(EXIT_FAILURE);
}

// print error indicated by errno and exit with code 1(FAILURE)
void perror_exit(char* message){
    perror(message);
    exit(EXIT_FAILURE);
}

// print a message in stderr and exit with code 1(FAILURE)
void error_exit(const char* format,...){
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    exit(EXIT_FAILURE);
}

// print a message in stderr and return $ret_val to the caller
int error_return(int ret_val, const char* format, ...){
    va_list args;
    va_start(args, format);
    vfprintf(stdout, format, args);
    va_end(args);
    return ret_val;
}

// lock logfile and print a message
int exclusive_print(int fd, const char* format, ...){

    // get arguments
    va_list args;
    va_start(args, format);

    // lock file,print message and flush to the disc
    flock(fd, LOCK_EX);
    vdprintf(fd, format, args);
    fsync(fd);
    flock(fd, LOCK_UN);
    va_end(args);
    return 0;
}

// in case of an error while syncing, the following code is executed
int sync_error(int ret_val, int sig, const char* format, ...){
    extern struct cmd args;
    extern int fd_copy;
    extern list_t list;

    // send a signal indicating the error
    kill(getppid(), sig);

    // print a message
    va_list parameters;
    va_start(parameters, format);
    vfprintf(stderr, format, parameters);
    va_end(parameters);

    // free all resources
    closedir(args.mirror); closedir(args.input); closedir(args.common);
    args.mirror = args.input = args.common = NULL;
    if(fd_copy >= 0)
        close(fd_copy);
    listFree(&list);
    return ret_val;
} 

// alarm_read, reads from a named pipe but only blocks for a certain amount of time
int alarm_read(int fd, void* buffer, size_t sz, unsigned int secs){
    int ret_val = -1;
    signal(SIGALRM, alarm_handler);

    alarm(secs); // activate alarm clock
    ret_val = read(fd, buffer, sz);
    alarm(0);    // de-activate the alarm clock 
    return ret_val;
}

/*------------------------------The following are member methods of a linked list storing data pf type struct SyncInfo-------------------*/ 
int assign(void* new_info, const void* old_info){
    if(new_info == NULL || old_info == NULL)
        return -1;
    struct SyncInfo *new = (struct SyncInfo*)new_info, *old = (struct SyncInfo*)old_info;
    new->pid = old->pid;
    new->id = old->id;
    new->code = old->code;
    new->tries = old->tries;
    return 0;
}

void *compare(void* info, const void* vpid){
    if(info == NULL || vpid == NULL)
        return NULL;
    struct SyncInfo *sync_info = (struct SyncInfo*)info;
    pid_t *pid = (pid_t*)vpid;
    if(sync_info->pid == *pid)
        return info;
    else 
        return NULL;
}

int print(void *info){
    if(info == NULL)
        return -1;
    struct SyncInfo *sync_info = (struct SyncInfo*)info;
    if(sync_info->code == 0)
        fprintf(stdout, "pid: %d\tsending to:\t%hhd\ttry: %hhd\n", (int)sync_info->pid, sync_info->id, sync_info->tries + 1);
    else
        fprintf(stdout, "pid: %d\treceiving from:\t%hhd\ttry: %hhd\n", (int)sync_info->pid, sync_info->id, sync_info->tries + 1);
    return 0;
}