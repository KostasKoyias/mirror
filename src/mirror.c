/****************************************************************************************************************************************\
 * mirror runs a client's code. First off, it creates a file in the common directory using the client's id as it's name, like a sign-up *
 * after that it copies all data of other users under his mirror directory creating a sub-directory for each user containing his files  *
 * and folders. Finally it monitors the system every now and then, inspecting possible changes, like new registrations or deletions     *
 * of users and makes sure the client's mirror directory is always up to date and in sync with the directories of other users           * 
\****************************************************************************************************************************************/
#include "headers/define.h"
#include "headers/utils.h"
#include "headers/cmd.h"
#include "headers/sync.h"
#include "../gen-list/list.h"

/* variables declared global, so that all signal handlers can use them */
struct cmd args;
char **argvPtr = NULL, fifo[MAX_DIR + 2*MAX_ID + 10];
int fd_copy = -1;
pid_t my_id;
list_t list = {NULL, sizeof(struct SyncInfo), 0, compare, assign, print, NULL, NULL};

int main(int argc, char* argv[]){
    /* declare variables*/
    int  id2, common_fd, status;
    char id_str[MAX_DIR + MAX_ID + 4], dirpath[MAX_BUFFER], pid_str[MAX_PID];
    struct dirent *direntp, *direntp2;
    struct sigaction action = {NULL};
    node_t *parser;
    struct SyncInfo *info;
    uint8_t flag = 0;
    my_id = getpid(); 

    // all signals except SIGALRM, SIGINT and SIGQUIT are blocked while in the handler's body
    sigfillset(&(action.sa_mask));
    sigdelset(&(action.sa_mask), SIGALRM);
    sigdelset(&(action.sa_mask), SIGINT);
    sigdelset(&(action.sa_mask), SIGQUIT);    
    action.sa_sigaction = handler;
    action.sa_flags = SA_SIGINFO;

    sigaction(SIGINT, &action, NULL);
    sigaction(SIGQUIT, &action, NULL);
    sigaction(SIGUSR1, &action, NULL);
    sigaction(SIGUSR2, &action, NULL);

    /* get cmd arguments, make sure usage is correct and all values within the appropriate range*/ 
    cmd_check(argc, argv, &args);
    argvPtr = argv;

    /* create client's private file in the common directory and insert his pid as a header*/
    sprintf(id_str, "%s/%d.id", argv[args.common_i], args.id);
    if((common_fd = open(id_str, O_RDWR | O_CREAT | O_EXCL, FILE_PERMS)) == -1){
        fprintf(stdout, "mirror: Error, %s exists or could not be created, exiting now...\n", id_str);
        free_rsrc(EXIT_FAILURE);
    }
    sprintf(pid_str, "%d", getpid());
    write(common_fd, pid_str, strlen(pid_str));
    close(common_fd);

    /*copy all input directories of other clients as well as their contents into this client's mirror directory*/
    fprintf(stdout, "***********************Initialization*********************\n");
    while((direntp = readdir(args.common)) != NULL){
        // omit current and parent directory as well as your own directory and *.fifo files
        if((strcmp("..", direntp->d_name) == 0) || (strcmp(".", direntp->d_name) == 0)\
        || ((id2 = atoi(strtok(direntp->d_name, "."))) == args.id) || strcmp(strtok(NULL, "."), "fifo") == 0)
            continue;
        fprintf(stdout, "%d: cathing up with: %s\n", args.id, direntp->d_name);
        sync_with(id2);
    }

    /*monitor the common directory and update the client's mirror directory adding new client's input and removing any deleted clients*/
    while(1){
        sleep(CYCLE);
        fprintf(stdout, "***********************Monitoring************************\n");

        // for each user in your mirror directory, make sure he still has a file in the common directory, otherwise he has left the system 
        rewinddir(args.mirror);
        while((direntp = readdir(args.mirror)) != NULL){
            // omit current and parent directory of mirror directory
            if((strcmp("..", direntp->d_name) == 0) || (strcmp(".", direntp->d_name) == 0))
                continue;

            flag = 0;
            rewinddir(args.common);
            while(flag == 0 && (direntp2 = readdir(args.common)) != NULL){
                // omit current and parent and directory, as well as your own file and *.fifo files 
                if(strstr(direntp2->d_name, ".fifo") == NULL && strlen(direntp2->d_name) > 2){
                    direntp2->d_name[strlen(direntp2->d_name)-3] = '\0'; // cut off the .id prefix
                    if(atoi(direntp2->d_name) == args.id)
                        continue;
                }
                else
                    continue;
                if(strcmp(direntp->d_name, direntp2->d_name) == 0)
                    flag = 1;
            }
            // if a user logged out, remove his input directory from the client's mirror directory
            if(flag == 0){
                fprintf(stdout, "%d: user %s left the system, removing him from my mirror directory\n", args.id, direntp->d_name);
                sprintf(dirpath, "%s/%s", argv[args.mirror_i], direntp->d_name);
                if(fork() == 0)
                    execlp("rm", "rm", "-rf", dirpath, NULL);
                wait(NULL);
            }
        }

        // for each user in the common directory, make sure a sub-folder exists under your mirror directory, otherwise he is a new user, catch up
        rewinddir(args.common);
        while((direntp = readdir(args.common)) != NULL){
            // omit current and parent directories as well as your own .id and *.fifo files
            if(strstr(direntp->d_name, ".fifo") == NULL && strlen(direntp->d_name) > 2){
                direntp->d_name[strlen(direntp->d_name)-3] = '\0'; // cut off the .id prefix
                if(atoi(direntp->d_name) == args.id)
                    continue;
            }
            else
                continue;

            flag = 0;
            rewinddir(args.mirror);
            while((direntp2 = readdir(args.mirror)) != NULL && flag == 0){
                if((strcmp("..", direntp2->d_name) == 0) || (strcmp(".", direntp2->d_name) == 0))
                    continue;
                if(strcmp(direntp->d_name, direntp2->d_name) == 0)
                    flag = 1;
            }
            // if a user we are not synced with yet is detected
            if(flag == 0){
                // if there no processes trying to sync with him/her at the moment, create two
                for(parser = list.head; parser != NULL; parser = parser->next){
                    info = (struct SyncInfo*)parser->data;
                    if(info->id == atoi(direntp->d_name)){
                        flag = 1;
                        fprintf(stdout, "%d: syncing operation with %s is in progress\n", args.id, direntp->d_name);
                        break;
                    }
                }
                if(flag == 0){
                    fprintf(stdout, "%d: new user %s detected\n", args.id, direntp->d_name);
                    sync_with(atoi(direntp->d_name));
                }
            }
            else 
                fprintf(stdout, "%d: already synced with %s\n", args.id, direntp->d_name);
        }  

        // while sleeping, children might have completed a transfer, collect their exit codes using waitpid
        for(parser = list.head; parser != NULL; parser = parser->next){
            info = (struct SyncInfo*)parser->data;
            if(waitpid(info->pid, &status, WNOHANG) > 0 && WIFEXITED(status) && WEXITSTATUS(status) == EXIT_SUCCESS){
                if(info->code == 0)
                    fprintf(stdout, "%d: All files were successfully send to %d\n", args.id, info->id);
                else 
                    fprintf(stdout, "%d: All files were successfully received from %d\n", args.id, info->id);
                listDelete(&list, &(info->pid));
            }
        }

        // display working list
        fprintf(stdout, "%c[4m%d: Children Operating at the moment\n%c[0m", 27, args.id, 27);
            listPrint(&list);
    }

    return 0;
}
/*-------------------------------------------free all resources used by the process-------------------------------------------*/
void free_rsrc(int exit_code){

    // close all directories  
    if(args.common != NULL){
        closedir(args.common);
        args.common = NULL;
    } 
    if(args.input != NULL){
        closedir(args.input); 
        args.input = NULL;
    }
    if(args.mirror != NULL){
        closedir(args.mirror);
        args.mirror = NULL;
    } 

    // close all files
    if(args.logfile >= 0)
        close(args.logfile); 

    if(fd_copy >= 0)
        close(fd_copy);

    listFree(&list);
    exit(exit_code);
}
/*------------------------------------in case of receiving a signal, handler will be executed------------------------------------*/
void handler(int sig, siginfo_t *siginfo, void *ucontext){
    char my_common[MAX_DIR + MAX_ID + 5];
    struct SyncInfo *info;
    void *result;

    // senders signal a SIGUSR1 to their parent in case of a failed transfer, parent tries again or gives up after the 3rd try 
    if(sig == SIGUSR1){
        if(siginfo == NULL)
            return;
        if((result = listSearch(&list, &(siginfo->si_pid))) == NULL){
            fprintf(stdout, "%d:\tprocess %d is not in my sending list\n", args.id, (int)siginfo->si_pid);
            return;
        }
        info = (struct SyncInfo*)result;
        fprintf(stdout, "%d:\treceived a SIGUSR1 signal, this was sending try no.%d\n", args.id, info->tries + 1);
        if((++info->tries) <= 2){
            fprintf(stdout, "\trestarting the sending process to %d\n", info->id);
            if((info->pid = fork()) == 0)
                exit(send_content(info->id));
        }
        else 
            listDelete(&list, &(info->pid));
    }

    // receivers signal a SIGUSR2 to their parent in case of a failed transfer, parent tries again or gives up after the 3rd try 
    else if(sig == SIGUSR2){
        if(siginfo == NULL)
            return;
        if((result = listSearch(&list, &(siginfo->si_pid))) == NULL){
            fprintf(stdout, "%d:\tprocess %d is not in my receiving list\n", args.id, (int)siginfo->si_pid);
            return;
        }
        info = (struct SyncInfo*)result;
        fprintf(stdout, "%d:\treceived a SIGUSR2 signal, this was receiving try no.%d\n", args.id, info->tries + 1);
        if((++info->tries) <= 2){
            fprintf(stdout, "\trestarting the receiving process from %d\n", info->id);
            if((info->pid = fork()) == 0)
                exit(recv_content(info->id));
        }
        else 
            listDelete(&list, &(info->pid));
    }
    
    //process can only terminate with a SIGINT or SIGQUIT, handler de-allocates memory used and calls exit with code 0
    else if(sig == SIGQUIT || sig == SIGINT){
        fprintf(stdout, "%d: %d received %s signal, exiting now...\n", args.id, (int)getpid(), strsignal(sig));
        sprintf(my_common, "%s/%d.id", argvPtr[args.common_i], args.id);

        // remove your id from common dir
        if(fork() == 0)
            execlp("rm", "rm", "-f", my_common, NULL);
        wait(NULL);

        // state that you left the system in your logfile, but only if you are the parent process
        if(my_id == getpid())
            dprintf(args.logfile, "%d\ta\n", args.id); //update logfile  
        free_rsrc(EXIT_SUCCESS);
    }
}