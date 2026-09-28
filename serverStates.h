#ifndef __SERVERSTATES_H__
#define __SERVERSTATES_H__

typedef enum {
    POLL, 
    SEND,
    READ,
    SREJ,
    END
} USESTATES;

// ---------- Includes ---------- //
#include <stdio.h>
#include <stdlib.h>

#include "server.h"
#include "cpe464.h"

// ---------- Function Prototypes ---------- //
void serverSetUp(int childSocket, char *recvBuffer, struct sockaddr_in6 *client, int *windowSize, int *bufferSize, char *fileName);
void serverUse(int childSocket, struct sockaddr_in6 *client, char **window, const int windowSize, const int bufferSize, const char *fileName);

// ---------- Macros ---------- //
#define PDUSIZE 1407
#define PAYLOADSIZE 1400
#define HEADERLENGTH 7

#endif

