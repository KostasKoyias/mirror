CC = gcc
CFLAGS = -g3 -Wall
HDRS = list.h
OBJS = list.o
SOURCES = main.c list.c

all: main $(OBJS)  

# main programm
main: main.c $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(SOURCES) $(LFLAGS)

# data structures and utilities	
list.o: list.c 
	$(CC) $(CFLAGS) -c $? $(LFLAGS)

clean: 
	rm -f $(OBJS) main 