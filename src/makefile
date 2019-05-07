CC = gcc
CFLAGS = -g3 -Wall
HDRS = define.h utils.h cmd.h sync.h list.h
OBJS = cmd.o utils.o sync.o list.o
LFLAGS = -lpthread -lrt
SOURCES = mirror.c cmd.c utils.c sync.c list.c

all: mirror $(OBJS)  

# main programm
mirror: mirror.c $(OBJS)
	$(CC) $(CFLAGS) -o mirror $(SOURCES) $(LFLAGS)

# data structures and utilities	
utils.o: utils.c 
	$(CC) $(CFLAGS) -c utils.c $(LFLAGS)

sync.o: sync.c cmd.o
	$(CC) $(CFLAGS) -c sync.c cmd.c $(LFLAGS)

list.o: list.c 
	$(CC) $(CFLAGS) -c list.c $(LFLAGS)

cmd.o: cmd.c 
	$(CC) $(CFLAGS) -c cmd.c $(LFLAGS)
reset:
	rm -rf CLUTTER* mirror_dir_* common_dir	
clean: 
	rm -rf $(OBJS) mirror common_dir CLUTTER* log* mirror_dir_*
 