CC = gcc
CFLAGS = -g3 -Wall
OBJS = cmd.o utils.o sync.o list.o
LFLAGS = -lpthread -lrt
SOURCES = mirror.c cmd.c utils.c sync.c ../gen-list/list.c

all: mirror $(OBJS)  

# main programm
mirror: mirror.c $(OBJS)
	$(CC) $(CFLAGS) -o mirror $(SOURCES) $(LFLAGS)

# data structures and utilities	
utils.o: utils.c 
	$(CC) $(CFLAGS) -c utils.c $(LFLAGS)

sync.o: sync.c cmd.o
	$(CC) $(CFLAGS) -c sync.c cmd.c $(LFLAGS)

list.o: ../gen-list/list.c 
	$(CC) $(CFLAGS) -c ../gen-list/list.c $(LFLAGS)

cmd.o: cmd.c 
	$(CC) $(CFLAGS) -c cmd.c $(LFLAGS)
reset:
	rm -rf CLUTTER* mirror_* common_dir	log*
clean: 
	rm -rf $(OBJS) mirror common_dir CLUTTER* log* mirror_dir_*
 