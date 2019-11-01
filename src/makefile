CC = gcc
BASE = $(shell pwd)
LIST = ../list_t
CFLAGS = -g3 -Wall -I$(BASE)/include -I$(BASE)/$(LIST)
OBJS = cmd.o utils.o sync.o list.o
LFLAGS = -lpthread -lrt
SOURCES = mirror.c cmd.c utils.c sync.c $(LIST)/list.c

all: mirror $(OBJS)  

# main programm
mirror: mirror.c $(OBJS)
	$(CC) $(CFLAGS) -o mirror $(SOURCES) $(LFLAGS)

# data structures and utilities	
utils.o: utils.c 
	$(CC) $(CFLAGS) -c utils.c

sync.o: sync.c cmd.o
	$(CC) $(CFLAGS) -c sync.c cmd.c

list.o: $(LIST)/list.c 
	$(CC) $(CFLAGS) -c $(LIST)/list.c

cmd.o: cmd.c 
	$(CC) $(CFLAGS) -c cmd.c

reset:
	rm -rf CLUTTER* mirror_* common_dir	log*
clean: 
	rm -rf $(OBJS) mirror common_dir CLUTTER* log* mirror_dir_*
 