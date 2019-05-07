#!/bin/bash
#------------------------------------------a simple function returning the minimum of two values------------------------------------------#
function min(){
    if [ $# -ne 2 ]
    then
        return -1
    else
        if [ $1 -lt $2 ]
        then 
            return $1
        else
            return $2
        fi
    fi
}

#------------------------------------------make sure the arguments number and their range is correct--------------------------------------#
if [ $# -ne 4 ]
then
    echo "Usage: $0  main_dir  files_num   dirs_num   level" 
    exit 1
elif [ $2 -lt 0 -o $3 -lt 0 -o $4 -lt 0 ]
then 
    echo error: args 2-4 should all be positive integers
    exit 2
fi

#-------------------------------------------------------create the input directory-------------------------------------------------------#
# if there is a directory/file under the same path ask for permission to remove its content/it
if [ -e $1 ]
then 
    if [ -d $1 ]
    then 
        echo "$1: directory exists, truncate(y/n)?"
    else
        echo "$1: file exists, remove(y/n)?"
    fi
    read resp
    while [ "$resp" != "n" ] && [ "$resp" != "y" ]
    do
        echo "(y/n)?"
        read resp
    done
    if [ "$resp" == "n" ]
    then 
        echo exiting now
        exit 3
    else
        rm -rf $1
    fi
fi
mkdir -p $1

#-----------------------------------------------------create inner directories-----------------------------------------------------#
dirs=0
while [ $dirs -lt $3 ]
do
    # start from the initial directory 
    path=$1
    left=$(echo "$3-$dirs"|bc)
    min $left $4
    depth=$?
    let dirs=$dirs+$depth

    # make a chain of min(dirs_left, max_levels) directories 
    for ((i=0 ; i < depth; i++))
    do
        #generate a random name for the directory, if it already exists generate another one until you get it
        code=1
        while [ $code -ne 0 ]
        do
            path=$path/$(cat /dev/urandom | tr -dc 'a-zA-Z0-9' | fold -w 32 | head -1 -c 8)
            mkdir $path
            code=$?
        done

    done    
done
#----------------------------------------------------fill directories with files----------------------------------------------------#
files=0

#until goal number of files is generated, create a random-content file under each directory including the main directory
while [ $files -lt $2 ]
do
    fortune > $1/$(cat /dev/urandom | tr -dc 'a-zA-Z0-9' | fold -w 32 | head -1 -c 8)
    let files=$files+1

    #for each file under the main directory
    for p in $(ls $1)
    do 
        if [ $files -ge $2 ]
        then 
            break
        fi
        path=$1

        #if it is a directory
        if [ -d $1/$p ]
        then 
            #create one file for each sub-folder down its chain until depth = min(files_left, max_levels) is reached
            let left=$2-$files
            path=$path/$p
            min $left $4
            depth=$? 

            for ((i=0 ; i < depth; i++))
            do
                #generate a random name for the file and initialize it with random content
                fortune > $path/$(cat /dev/urandom | tr -dc 'a-zA-Z0-9' | fold -w 32 | head -1 -c 8)

                #get to the next directory down the chain(each directory has only one subdirectory), if there is none break
                flag=0
                for p1 in $(ls $path)
                do 
                    if [ -d $path/$p1 ]
                    then 
                        flag=1
                        path=$path/$p1
                    fi
                done
                if [ $flag -eq 0 ]
                then 
                    break
                fi
            done
            let files=$files+$i+1   
        fi 
    done 
done
echo $3 random input directories were created and initialized with $2 random files at most $4 levels deep, under main folder $1! 
tree $1
if [ $? -ne 0 ]
then 
    echo "install 'tree' and re-run the script to better visualize the result"
    ls -R $1
fi

            



