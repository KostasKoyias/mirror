/******************************************************************************************************************************************\
 * The following methods achieve synchronization between two clients, both of which have an input folder, containing their own files and  *
 * a mirror folder containing one sub-folder for every other client, a clone of the other client's input, that way all of them are synced *
\******************************************************************************************************************************************/

#include "headers/sync.h"
int who = 0;

/*-------------------------in case of an ALARM signal while receiving files the following code will be executed-------------------------*/
void alarm_handler(int signum){
    extern char fifo[MAX_DIR + 2*MAX_ID + 10];
    if(who == 1){
        sync_error(0, SIGUSR1, "%d: waiting %d seconds to open a named-pipe in order to send data, abort now...\n", (int)getpid(), MAX_WAIT_WRITE);
        if(fork() == 0)
            execlp("rm", "rm", "-rf", fifo, NULL);
    }
    else
        sync_error(0, SIGUSR2, "%d: waiting %d seconds to read from a named-pipe, abort now...\n", (int)getpid(), MAX_WAIT_READ);
    exit(EXIT_FAILURE);
} 

/*------------------copy all content of id1 to a named-pipe, returns 0 or success or a negative integer in case of failure------------------*/
int send_content(int id2){
    extern char fifo[MAX_DIR + 2*MAX_ID + 10];
    extern char **argvPtr;
    int fd_fifo, ret_val = 0;
    extern struct cmd args;
    signal(SIGALRM, alarm_handler);
    who = 1;

    // open or create a named_pipe in common dir named 'id1_to_id2.fifo'
    sprintf(fifo, "%s/%d_to_%d.fifo", argvPtr[args.common_i], args.id, id2);
    fprintf(stdout, "%d: About to send folder %s to %d via %s\n", args.id, argvPtr[args.input_i], id2, fifo);

    // if named_pipe does not exist, create it, if this is not possible or if it can then not be opened send a SIGUSR1 signal to your parent
    alarm(MAX_WAIT_WRITE);
    if((mkfifo(fifo, FILE_PERMS) == -1 && errno != EEXIST) || ((fd_fifo = open(fifo, O_WRONLY)) == -1)){
        kill(getppid(), SIGUSR1);
        perror("fifo");
        return error_return(1, "%d: Error, pipe %s can either not be created or opened\n", args.id, fifo); 
    }
    alarm(0);

    rewinddir(args.input);
    ret_val = send_folder(argvPtr[args.input_i], args.input, fd_fifo, fifo);
    close(fd_fifo);
    free_rsrc(ret_val);
    return ret_val;
}

/*------------------copy all folder's content into a named-pipe, use recursion in case of sub-folders, returns 0 in success------------------*/
int send_folder(const char* folderpath, DIR *dirPtr, int fd_fifo, const char* named_fifo){
    extern struct cmd args;
    char name[3], size[5], buffer[args.buffer_sz];
    char path[MAX_BUFFER];
    extern int fd_copy;
    struct dirent *direntPtr;
    struct stat st;
    int bytes;
    DIR *nested;

    // write all input files or directories recursively into the pipe
    while((direntPtr = readdir(dirPtr)) != NULL){
        // omit current and parent directory, so do not send those
        if(strcmp("..", direntPtr->d_name) == 0 || strcmp(".", direntPtr->d_name) == 0)
            continue;

        // open the file to be copied, then state it
        sprintf(path, "%s/%s", folderpath, direntPtr->d_name);   // path holds the relative path to the file or directory(e.g 'input/file')
        stat(path, &st);  // st contains input file's metadata

        // write file's or folder's PATH in the fifo, but first write it's LENGTH
        sprintf(name, "%.2d", (int)strlen(direntPtr->d_name)); // name holds 2 bytes indicating the file's name length
        write(fd_fifo, name, 2); 
        write(fd_fifo, direntPtr->d_name, (size_t)strlen(direntPtr->d_name));

        // if it is a folder, do the same thing recursively
        if(S_ISDIR(st.st_mode)){
            //inform the receiver that you are about to send a folder
            write(fd_fifo, "1", 1);
            if(((nested = opendir(path)) == NULL) || (send_folder(path, nested, fd_fifo, named_fifo) != 0)){
                closedir(nested);
                return sync_error(2, SIGUSR1, "%d: Error sending sub-directory '%s'\n", args.id, path); 
            }
            closedir(nested);
            continue;
        }
        // else it is a regular file, so let the receiver know, open and the send the file content
        write(fd_fifo, "0", 1);
        if((fd_copy = open(path, FILE_PERMS)) == -1)
            return sync_error(3, SIGUSR1, "%d: Error while opening file to be sent '%s'\n", args.id, path); 

        // write file's CONTENT in the fifo, but first write it's SIZE
        sprintf(size, "%.4d", (int)st.st_size);
        write(fd_fifo, size, 4);

        // read at most args.buffer_sz bytes at a time from input/file and write to the named_pipe
        while((bytes = read(fd_copy, buffer, args.buffer_sz)) == args.buffer_sz){
            if(write(fd_fifo, buffer, args.buffer_sz) != args.buffer_sz)
                return sync_error(4, SIGUSR1, "%d: Error while transfering file '%s' to pipe '%s\n", args.id, path, named_fifo); 
        }
        if(bytes == -1)
            return sync_error(5, SIGUSR1, "%d: Error while reading from the input file %s\n", args.id, path); 
        write(fd_fifo, buffer, bytes); //write remaining bytes
        exclusive_print(args.logfile, "%d\ts\t%d\t%s\n", args.id, (int)st.st_size, path); //update logfile
    }
    write(fd_fifo, "00", 2);  // indicate that the input is over placing "00" at the pipe
    return 0;
}

/*----------------------------------synchronize a client's mirror directory with the input of another----------------------------------*/
int recv_content(int id2){
    extern char **argvPtr;
    extern struct cmd args;
    char fifo[2*MAX_FILE], dirname[MAX_BUFFER];
    int fd_fifo, ret_val;
    who = 2;
    signal(SIGALRM, alarm_handler);

    // open or create a named_pipe in common dir named 'id2_to_id1.fifo'
    sprintf(fifo, "%s/%d_to_%d.fifo", argvPtr[args.common_i], id2, args.id);
    fprintf(stdout, "%d: About to receive files from %d via %s\n",  args.id, id2, fifo);

    // if named_pipe does not exist, create it, if this is not possible or if it can then not be opened send a SIGUSR2 signal to your parent
    alarm(MAX_WAIT_READ); // do not get blocked for more than MAX_WAIT_READ secs
    if((mkfifo(fifo, FILE_PERMS) == -1 && errno != EEXIST) || ((fd_fifo = open(fifo, O_RDONLY)) == -1)){
        kill(getppid(), SIGUSR2);
        return error_return(1, "%d: Error, pipe %s can either not be created or opened\n", args.id, fifo); 
    }
    alarm(0);       // re-set the alarm


    //create mirror folder of type 'mirror_dir/id2' and copy all files of id2 in there
    sprintf(dirname, "%s/%d", argvPtr[args.mirror_i], id2);
    ret_val = recv_folder(dirname, fd_fifo, fifo);
    close(fd_fifo);
    if(fork() == 0)
        execlp("rm", "rm", "-rf", fifo, NULL);
    free_rsrc(ret_val);   
    return ret_val;
}

/*---------this function clones a folder recursively, returns 0 in case of success or a negative integer indicating what went wrong----------*/ 
int recv_folder(const char* dirname, int fd_fifo, const char* named_fifo){
    extern struct cmd args;
    int bytes_sz, bytes;
    extern int fd_copy;
    char isdir[2], name[3], size[5], filename[MAX_FILE], buffer[args.buffer_sz], path[MAX_BUFFER];
    isdir[1] = name[2] = size[4] = '\0';

    //create destination folder
    if(mkdir(dirname, DIR_PERMS) == -1){
        kill(getppid(), SIGUSR2);
        perror("mkdir");
        return error_return(3, "%d: Error making mirror directory '%s'\n", args.id, dirname); 
    }

    //read from pipe until "00" is encountered 
    while((alarm_read(fd_fifo, name, 2, MAX_WAIT_READ) == 2) && (strcmp(name, "00") != 0)){ //get length of file's name
        alarm_read(fd_fifo, filename, atoi(name), MAX_WAIT_READ); //read the name
        filename[atoi(name)] = '\0';

        //create a file or folder to copy the other client's file content
        sprintf(path, "%s/%s", dirname, filename); 

        //if a folder is being sent make one
        alarm_read(fd_fifo, isdir, 1, MAX_WAIT_READ);
        if(atoi(isdir) == 1){
            if(recv_folder(path, fd_fifo, named_fifo) != 0)
                return sync_error(4, SIGUSR2, "%d: Error receiving sub-directory '%s'\n", args.id, path); 
            continue;
        }
        // else if it is a file create one
        if((fd_copy = open(path, O_CREAT | O_WRONLY | O_TRUNC, FILE_PERMS)) == -1)
            return sync_error(5, SIGUSR2, "%d: Error making mirror file '%s'\n", args.id, path); 

        // get file size in bytes
        if(alarm_read(fd_fifo, size, 4, MAX_WAIT_READ) != 4)
            return sync_error(6, SIGUSR2, "%d: Error getting %s's size in bytesfrom %s\n", args.id, filename, named_fifo);
        bytes_sz = atoi(size);

        // read from the named_pipe at most args.buffer_sz bytes and write them to the mirror file, bytes_sz is what is left
        while((bytes = alarm_read(fd_fifo, buffer, MIN(args.buffer_sz, bytes_sz), MAX_WAIT_READ)) > 0){
            write(fd_copy, buffer, bytes);
            bytes_sz -= bytes;
        }   
        exclusive_print(args.logfile, "%d\tr\t%d\t%s\n", args.id, atoi(size), path); //update logfile  
    }

    // if communication ended based on the protocol, return 0, else 7
    if(strcmp(name, "00") == 0)
        return 0;
    else 
        return 7;   
}

/*---------------------------------synchronize with another user creating a sending and a receiving process-------------------*/
int sync_with(int id2){
    struct SyncInfo info;
    extern struct G_list list;
    info.id = id2;
    info.tries = 0;

    // start a sending process and insert the operation's details in the working list
    if((info.pid = fork()) == 0)
        exit(send_content(id2));
    else if(info.pid < 0){
        perror("sync_with: sending fork");
        return -1;
    }
    info.code = 0;
    listInsert(&list, &info);

    // start a receiving process and insert the operation's details in the working list
    if((info.pid = fork()) == 0)
        exit(recv_content(id2));
    else if(info.pid < 0){
        perror("sync_with: receiving fork");
        return -2;
    }
    info.code = 1;
    listInsert(&list, &info);
    return 0;
}
