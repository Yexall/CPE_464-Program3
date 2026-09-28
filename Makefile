# Makefile for CPE464 UDP test code
# Written by Hugh Smith - April 2019
# Modified by Yexelle Nash Segovia - March 2025

LIBS = 
OBJS = networks.o gethostbyname.o pollLib.o safeUtil.o clientTable.o PDU.o serverStates.o windowBuffer.o rcopyStates.o
CFLAGS = -g -Wall -Wextra -pedantic -std=gnu99 

LIBS += libcpe464.2.21.a -lstdc++ -ldl
CFLAGS += -D__LIBCPE464_

all:   rcopy server

rcopy: rcopy.c rcopy.h $(OBJS)
	gcc $(CFLAGS) rcopy.c -o rcopy  $(OBJS) $(LIBS)

server: server.c server.h $(OBJS)
	gcc $(CFLAGS) server.c -o server $(OBJS) $(LIBS)

.c.o:
	gcc -c $(CFLAGS) $< -o $@ $(LIBS)

cleano:
	rm -f *.o

clean:
	rm -f rcopy server *.o




