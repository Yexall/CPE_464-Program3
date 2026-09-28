#ifndef __RCOPYSTATES_H__
#define __RCOPYSTATES_H__

typedef enum {
    POLL, 
    READ,
    SEND,
    BUFFER,
    FLUSH
} USESTATES;

// ---------- Includes ---------- //
#include <stdio.h>
#include <stdlib.h>

#include "PDU.h"
#include "rcopy.h"
#include "cpe464.h"

// ---------- Function Prototypes ---------- //
void rcopySetUp(int clientSocket, struct sockaddr_in6 *server, int windowSize, int bufferSize, char *fromFileName);
void rcopyUse(int clientSocket, struct sockaddr_in6 *server, char **window, int windowSize, char *toFileName);

// ---------- Macros ---------- //
#define PDUSIZE 1407
#define PAYLOADSIZE 1400
#define HEADERLENGTH 7

#endif

