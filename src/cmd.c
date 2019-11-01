 /********************************************************************************************************************\
  * cmd handles the command line arguments, making sure they are of the right format and within the appropriate range* 
  * It also stores the index of each directory passed and opens all directories and files mirror needs.              *
\*********************************************************************************************************************/
 
 #include "cmd.h"

 /*-------make sure all cmd arguments exist and are within the appropriate range, return common directory's index in the argument vector----*/
 void cmd_check(int argc, char** argv, struct cmd *cmd_args){
    int i;
    *cmd_args = (struct cmd){.common = NULL, .input = NULL, .mirror = NULL, .id = -1,\
    .buffer_sz = -1, .logfile = -1, .common_i = 0, .input_i = 0, .mirror_i = 0, .rm_mirror = 0};

    if(argc < 13 || argc > 14)
        usage_error();

    // for each flag detected, store the index of the argument passed 
    for(i = 1; i < argc-1; i+=2){
        if(strcmp(argv[i], "-n") == 0)
            cmd_args->id = atoi(argv[i+1]); 
        else if(strcmp(argv[i], "-i") == 0)
            cmd_args->input_i = i+1;             
        else if(strcmp(argv[i], "-c") == 0)
            cmd_args->common_i = i+1;
        else if(strcmp(argv[i], "-m") == 0)
            cmd_args->mirror_i = i+1;
        else if(strcmp(argv[i], "-l") == 0)
            cmd_args->logfile = i+1;
        else if(strcmp(argv[i], "-b") == 0)
            cmd_args->buffer_sz = atoi(argv[i+1]);
        else 
            usage_error();
    }

    // check whether a cleanup was requested, it shall be the last argument
    if(strcmp(argv[argc-1], "-rm") == 0)
            cmd_args->rm_mirror = 1;

    // if any of the 6 mandatory arguments was omitted, abort 
    if(cmd_args->id == -1 || cmd_args->logfile == -1 || cmd_args->buffer_sz == -1 || cmd_args->common_i == 0 || cmd_args->input_i == 0 || cmd_args->mirror_i == 0)
        usage_error();

    /*---------------------------Check whether all arguments are valid and within the appropriate range---------------------------*/
    
    // id needs to be a positive integer
    if(cmd_args->id <= 0)
        error_exit("mirror: Id %d is invalid, exiting now...\n", cmd_args->id);

    // input dir must exist
    if((cmd_args->input = opendir(argv[cmd_args->input_i])) == NULL)
        perror_exit("mirror: input_dir error");
    
    // open or create and then open common_dir;
    if((cmd_args->common = opendir(argv[cmd_args->common_i])) == NULL){
        // if it does not exist create and open it
        if((mkdir(argv[cmd_args->common_i], DIR_PERMS) == -1) || ((cmd_args->common = opendir(argv[cmd_args->common_i])) == NULL)){
            closedir(cmd_args->input);
            perror_exit("mirror: mkdir/open common_dir error");
        }
    }

    // create a mirror dir, storing all content of other clients
    if(mkdir(argv[cmd_args->mirror_i], DIR_PERMS) != 0){
        closedir(cmd_args->input); closedir(cmd_args->common);
        // if mirror_dir exists or could not be created, abort
        if(errno == EEXIST)
            error_exit("mirror: file/dir '%s' already exists, another client might be using it, exiting now...\n", argv[cmd_args->mirror_i]);
        else
            perror_exit("mirror: creating mirror_dir");
    }
    // if it does not exist already, open mirror_dir
    else if((cmd_args->mirror = opendir(argv[cmd_args->mirror_i])) == NULL)
        perror_exit("mirror: opening mirror_dir");
    
    // create the log file, all previous content is deleted
    if((cmd_args->logfile = open(argv[cmd_args->logfile], O_WRONLY | O_CREAT | O_TRUNC, FILE_PERMS)) == -1){
        closedir(cmd_args->input); 
        closedir(cmd_args->mirror); rmdir(argv[cmd_args->mirror_i]); 
        closedir(cmd_args->common);
        perror_exit("mirror: open logfile");
    }
    
    // make sure buffer size given is within the appropriate range
    if(cmd_args->buffer_sz <=0 || cmd_args->buffer_sz > MAX_BUFFER)
        error_exit("mirror: buffer size %d is out of range [1, %d], exiting now...\n", cmd_args->buffer_sz, MAX_BUFFER);
}

 