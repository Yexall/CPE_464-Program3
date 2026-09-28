#include "serverStates.h"

void serverSetUp(int childSocket, char *recvBuffer, struct sockaddr_in6 *client, int *windowSize, int *bufferSize, char *fileName) {
    // Receive variables //
    int clientAddrLength = sizeof(struct sockaddr_in6);

    sscanf(recvBuffer + HEADERLENGTH, "%d %d %s", windowSize, bufferSize, fileName);   //Extract windowSize, bufferSize and fileName

    // Send variables //
    int totalLength = 0;
    uint16_t checksum;
    char PDU [PDUSIZE + 1], payload[PAYLOADSIZE + 1];

    // Attempt to open fileName. Send an ACK if successful, ERROR if not //
    FILE* file;
    file = fopen(fileName, "r");

    if (file == NULL) {                                                     //File could not be opened
        sprintf(payload, "ERROR: File not found\n");
        createPDU(PDU, payload, -1, 9);
        totalLength = strlen(payload) + HEADERLENGTH;
        checksum = in_cksum((unsigned short *)(PDU), totalLength);
        memcpy(PDU + 4, &checksum, 2);
        sendtoErr(childSocket, PDU, totalLength, 0, (struct sockaddr *) client, clientAddrLength);
        
    } else {                                                                //File could be opened
        sprintf(payload, "ACK\n");
        createPDU(PDU, payload, -1, 9);
        totalLength = strlen(payload) + HEADERLENGTH;
        checksum = in_cksum((unsigned short *)(PDU), totalLength);
        memcpy(PDU + 4, &checksum, 2);
        sendtoErr(childSocket, PDU, totalLength, 0, (struct sockaddr *) client, clientAddrLength);
        fclose(file);

    }
}

void serverUse(int childSocket, struct sockaddr_in6 *client, char **window, const int windowSize, const int bufferSize, const char *fileName) {
    // Send variables //
    int totalLength = 0, sequenceNumber = 0;
    uint16_t checksum;
    char sendPDU[PDUSIZE + 1];

    // Resend variables //
    int index = 0, totalResendLength = 0;
    char resendPDU[PDUSIZE + 1];
    
    // Receive variables //
    int flag = 0, recvLength = 0, clientAddrLength = sizeof(struct sockaddr_in6);
    uint32_t rrNumHost = 0, rrNumNetwork = 0;
    char recvPDU[PDUSIZE + 1];

    // Window variables //
    int lowest = 0, current = 0, highest = lowest + windowSize;

    // File reading variables //
    int bytesRead = 0, bufferLength = 0;
    char buffer[bufferSize + 1];
    FILE* file = fopen(fileName, "r");

    // Set up polling //
	int pollTime = 0, pollResult = 0;
	setupPollSet();
	addToPollSet(childSocket);				    //Add child socket to pollset	
    
    int timeoutCounter = 0;
    USESTATES state = SEND;                     //Set initial state

	while (1) {
        switch (state) {
            case POLL:
                pollResult = pollCall(pollTime);

                if (timeoutCounter == 10) {
                    printf("Client unresponsive. Ending connection.\n");
                    close(childSocket);
                    
                    return;
                }

                if (pollResult > 0) {
                    pollTime = 0;                  //Reset polling
                    timeoutCounter = 0;            //Reset counter

                    state = READ;
                } else {                    
                    // Find the lowest packet in the window //
                    index = lowest % windowSize;
                    totalResendLength = strlen(window[index]) + HEADERLENGTH;

                    // Send lowest packet in the window to rcopy //
                    createPDU(resendPDU, window[index], lowest, 18);
                    checksum = in_cksum((unsigned short *)(resendPDU), totalResendLength);
                    memcpy(resendPDU + 4, &checksum, 2);
                    sendtoErr(childSocket, resendPDU, totalResendLength, 0, (struct sockaddr *) client, clientAddrLength);

                    pollTime = 1000;
                    ++timeoutCounter;
                }
                break;
            case READ:
                recvLength = safeRecvfrom(childSocket, recvPDU, PDUSIZE, 0, (struct sockaddr *) client, &clientAddrLength);
                
                if (in_cksum((unsigned short *)(recvPDU), recvLength) == 0) {
                    flag = recvPDU[6];                                          //Set flag

                    memcpy(&rrNumNetwork, recvPDU, 4);                          //Copy RRNumber
                    rrNumHost = ntohl(rrNumNetwork);                            //Convert to host

                    if (flag == 5) {
                        // Update window variables //
                        lowest = rrNumHost;
                        highest = lowest + windowSize;
                        
                        state = SEND;
                    } else if (flag == 6) {
                        
                        state = SREJ;
                    } else {
                        fprintf(stderr, "ERROR: Unknown flag received: %d\n", flag);
                        state = POLL;

                    }
                } else {
                    state = POLL;
                }
                break;
            case SEND:              //Read data and send to rcopy 
                if ((bytesRead = fread(buffer, sizeof(char), bufferSize, file)) > 0) {
                    buffer[bytesRead] = '\0';   //Null terminate the buffer

                    bufferLength = bytesRead;
                    totalLength = bufferLength + HEADERLENGTH;
                    
                    // Send to rcopy //
                    createPDU(sendPDU, buffer, sequenceNumber, 16);
                    checksum = in_cksum((unsigned short *)(sendPDU), totalLength);
                    memcpy(sendPDU + 4, &checksum, 2);
                    sendtoErr(childSocket, sendPDU, totalLength, 0, (struct sockaddr *) client, clientAddrLength);

                    addToWindow(window, sequenceNumber, buffer, windowSize, bufferLength);  //Add packet to window buffer
                    
                    // Increment //
                    ++current;
                    ++sequenceNumber;

                    if (current == highest) {
                        pollTime = 1000;
                        timeoutCounter = 0;

                        state = POLL;
                    } else if (current < highest) {
                        state = SEND;
    
                    }
                } else {
                    state = END;

                }
                break;
            case SREJ:
                index = rrNumHost % windowSize;
                totalResendLength = strlen(window[index]) + HEADERLENGTH;

                // Send to rcopy //
                createPDU(resendPDU, window[index], rrNumHost, 17);
                checksum = in_cksum((unsigned short *)(resendPDU), totalResendLength);
                memcpy(resendPDU + 4, &checksum, 2);
                sendtoErr(childSocket, resendPDU, totalResendLength, 0, (struct sockaddr *) client, clientAddrLength);

                pollTime = 1000;            //Poll for 1 second
                ++timeoutCounter;           //Increment counter
                state = POLL;
                break;
            case END:
                // Send EOF to rcopy //
                createPDU(sendPDU, buffer, sequenceNumber, 10);
                checksum = in_cksum((unsigned short *)(sendPDU), totalLength);
                memcpy(sendPDU + 4, &checksum, 2);
                sendtoErr(childSocket, sendPDU, totalLength, 0, (struct sockaddr *) client, clientAddrLength);

                state = POLL;
                break;
            default:
                break;
        }
    }
}
