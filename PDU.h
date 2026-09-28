#ifndef __PDU_H__
#define __PDU_H__

// ---------- Includes ---------- //
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>         //For exit()
#include <string.h>         //For memcpy()
#include <arpa/inet.h>      //For ntohs()

#include "safeUtil.h"

// ---------- Function Prototypes ---------- //
void createPDU(char *PDU, char *payload, uint32_t packetSequence, int flag);

// ---------- Macros ---------- //
#define PDUSIZE 1407

#endif

