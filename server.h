#ifndef __SERVER_H__
#define __SERVER_H__

typedef enum {
    SETUP,
    USE,
    TEARDOWN
} STATES;

// ---------- Includes ---------- //
#include <stdio.h>
#include <stdlib.h>

#include "PDU.h"
#include "cpe464.h"
#include "pollLib.h"
#include "networks.h"
#include "safeUtil.h"
#include "clientTable.h"
#include "windowBuffer.h"
#include "serverStates.h"

// ---------- Function Prototypes ---------- //
int checkArgs(int argc, char *argv[]);
void serverControl(int serverSocket, double errorRate);

// ---------- Macros ---------- //
#define PDUSIZE 1407
#define DEBUG_FLAG 1

#endif

