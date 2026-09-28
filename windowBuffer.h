#ifndef __WINDOWBUFFER_H__
#define __WINDOWBUFFER_H__

// ---------- Includes ---------- //
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

// ---------- Function Prototypes ---------- //
char **initializeWindow(int windowSize);
void addToWindow(char **window, uint32_t sequenceNumber,char *payload, int windowSize, int payloadSize);

// ---------- Macros ---------- //
#define PAYLOADSIZE 1400

#endif