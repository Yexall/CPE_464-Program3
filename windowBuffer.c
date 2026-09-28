#include "windowBuffer.h"

char **initializeWindow(int windowSize) {
    char **window = malloc(windowSize * sizeof(char *));

    if (window == NULL) {                   //Check if malloc succeeded
        perror("ERROR: Malloc failed for window.\n");
        exit(1);
    
    }

    for (int i = 0; i < windowSize; ++i) {
        window[i] = malloc(PAYLOADSIZE + 1);
        memset(window[i], 0, PAYLOADSIZE + 1);

    }
    return window;
}

void addToWindow(char **window, uint32_t sequenceNumber, char *payload, int windowSize, int payloadSize) {
    int index = sequenceNumber % windowSize;

    memcpy(window[index], payload, payloadSize);    //Copy payload into window
    window[index][payloadSize] = '\0';              //Null terminate
}
