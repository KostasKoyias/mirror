#!/bin/bash

### argument check
if [ $# -ne 2 ]
then 
    echo "Usage: $0    number_of_names    name_length"
    exit 1
elif [ $1 -lt 1 -o $2 -lt 1 ]

### argument range check
then
    echo "Error: args should be positive"
    exit 2
fi

### generate $names number of names with $name_length random characters each
let names=$1
let len=$2 

### get all characters capital or not, in a list
declare -a chars 
for i in {a..z}
do 
    chars+=($i)
done 
for i in {A..Z}
do 
    chars+=($i)
done 

### create random names
i=1
while [ $i -le $names ] 
do 
    j=1
    name=""
    while [ $j -le $len ]
    do 
        let index=$RANDOM%52
        name="$name${chars[$index]}"
        ((j++))
    done
    echo $name
    ((i++))
done