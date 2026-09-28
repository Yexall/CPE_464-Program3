#ifndef __RCOPY_H__
#define __RCOPY_H__

typedef enum {
    SETUP,
    USE,
    TEARDOWN
} STATES;

// ---------- Includes ---------- //
#include <stdio.h>
#include <stdlib.h>

#include "cpe464.h"
#include "pollLib.h"
#include "networks.h"
#include "safeUtil.h"
#include "rcopyStates.h"
#include "windowBuffer.h"

// ---------- Function Prototypes ---------- //
int checkArgs(int argc, char *argv[]);
void clientControl(int clientSocket, struct sockaddr_in6 *server, int windowSize, int bufferSize, char *fromFileName, char *toFileName);
int readFromStdin(char *buffer);

// ---------- Macros ---------- //
#define PAYLOADSIZE 1400
#define DEBUG_FLAG 1

#endif

