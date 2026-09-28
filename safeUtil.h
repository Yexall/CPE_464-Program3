// 
// Writen by Hugh Smith, Jan. 2023
//
// Put in system calls with error checking.

#ifndef __SAFEUTIL_H__
#define __SAFEUTIL_H__

// ---------- Includes ---------- //
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/uio.h>
#include <sys/time.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>

#include "networks.h"

// ---------- Function Prototypes ---------- //
int safeRecvfrom(int socketNum, void * buf, int len, int flags, struct sockaddr *srcAddr, int * addrLen);
int safeSendto(int socketNum, void * buf, int len, int flags, struct sockaddr *srcAddr, int addrLen);

int safeRecv(int socketNum, uint8_t * buffer, int bufferLen, int flag);
int safeSend(int socketNum, uint8_t * buffer, int bufferLen, int flag);

void * srealloc(void *ptr, size_t size);
void * sCalloc(size_t nmemb, size_t size);

#endif