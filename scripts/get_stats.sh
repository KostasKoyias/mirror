#!/bin/bash

# declare variables
send_bytes=0
recv_bytes=0
send_files=0
recv_files=0
min_id=-1
max_id=-1
abort=0
declare -a list

# read file line by line
while read line
do 

    # for each line, make sure it is of the right format, else ignore
    args=($line)
    if [ ${#args[@]} -lt 2 ]
    then
        echo "Error: wrong line format, ignored, it should be: id  code  (bytes) ..."
        continue
    fi

    # update bytes sent/received or clients leaving the system depending on the line code[s(end),r(eceive) or a(bort)]
    code=${args[1]}
    if [ $code == "a" ]   #abort
    then
        ((abort++))
        continue
    elif [ $code == "s" ] #send
    then 
        ((send_bytes+=${args[2]}))
        ((send_files++))
    elif [ $code == "r" ] #receive
    then
        ((recv_bytes+=${args[2]}))
        ((recv_files++))
    else                  #error
        echo "Error: code should be in {s,r,a}, but $code was given"
    fi

    # update the id list, as well as the maximum and minimum id
    id=${args[0]}
    if [ ${#list[@]} -eq 0 ]
    then 
        min_id=$id
        max_id=$id
    elif [ $id -gt $max_id ]
    then
        max_id=$id
    elif [ $id -lt $min_id ]
    then 
        min_id=$id
    else 
        exists=0
        for i in ${list[@]}
        do 
            if [ $id -eq $i ]
            then 
                exists=1
                break
            fi
        done 
        if [ $exists -eq 1 ]
        then 
            continue 
        fi 
    fi
    list+=( $id)
done
echo -e "\e[1mids\t\n---\e[0m"
printf '%s\n' "${list[@]}" | sort -n
echo -e "\n\e[1mmax_id:\t\e[0m$max_id\n\e[1mmin_id:\t\e[0m$min_id\n\n\
\e[1mbytes_sent:\t\e[0m$send_bytes\n\e[1mbytes_recv:\t\e[0m$recv_bytes\n\n\
\e[1mfiles_sent:\t\e[0m$send_files\n\e[1mfiles_received:\t\e[0m$recv_files\n\n\e[1mleft system:\t\e[0m$abort"  