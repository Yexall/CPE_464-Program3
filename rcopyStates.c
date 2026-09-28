#include "rcopyStates.h"

void rcopySetUp(int clientSocket, struct sockaddr_in6 *server, int windowSize, int bufferSize, char *fromFileName) {
    // Send variables //
    int totalLength = 0;
    uint16_t checksum;
    char PDU [PDUSIZE + 1], payload[PAYLOADSIZE + 1];
	int serverAddrLength = sizeof(struct sockaddr_in6);

    // Send file name to server //
	sprintf(payload, "%d %d %s\n", windowSize, bufferSize, fromFileName);
    createPDU(PDU, payload, 0, 8);
    totalLength = strlen(payload) + HEADERLENGTH;
    checksum = in_cksum((unsigned short *)(PDU), totalLength);
    memcpy(PDU + 4, &checksum, 2);
	sendtoErr(clientSocket, PDU, totalLength, 0, (struct sockaddr *) server, serverAddrLength);

    // Receive variables //
    int flag = 0, recvLength = 0, payloadLength = 0, timeoutCounter = 0;
    char recvBuffer[PDUSIZE + 1], recvMessage[100 + 1]; 
    
    // Set up polling //
    int pollResult, pollTime = 0;
	setupPollSet();
	addToPollSet(clientSocket);

    while (1) {
        pollResult = pollCall(pollTime);

        if (timeoutCounter > 10) {     //10 seconds has passed. Terminate connection with server
            fprintf(stderr, "ERROR: Client timed out waiting for a response. Terminating...\n");
            exit(1);

        }

        if (pollResult > 0) {
            // Receive acknowledgment from server //
            recvLength = safeRecvfrom(clientSocket, recvBuffer, PDUSIZE, 0, (struct sockaddr *) server, &serverAddrLength);
            
            if (in_cksum((unsigned short *)(recvBuffer), recvLength) == 0) {
                flag = recvBuffer[6];                                           //Set flag
            
                payloadLength = recvLength - HEADERLENGTH;
                memcpy(recvMessage, recvBuffer + HEADERLENGTH, payloadLength);  //Copy payload
                recvMessage[payloadLength] = '\0';                              //Null terminate

                if (flag == 9) {                //Server's response from filename packet: ACK or ERROR
                    if (strncmp(recvMessage, "ACK", 3) == 0) {
                        return;

                    } else {
                        fprintf(stderr, "%s\n", recvMessage);
                        exit(1);

                    }
                } else if (flag == 16) {        //Server's ACK was lost
                    return;

                } else {                        //Error if the flag for a filename response != 9 
                    fprintf(stderr, "ERROR: Incorrect format for the filename packet. Terminating client.\n");
                    exit(1);

                }
            }
        } else {                            //In the event a response was not received within a given time            
            pollTime = 1000;                //Poll for 1 second
            ++timeoutCounter;               //Increment counter

        }
    }
}

void rcopyUse(int clientSocket, struct sockaddr_in6 *server, char **window, int windowSize, char *toFileName) {
    // Send variables //
    int totalLength = 0;
    uint16_t checksum;
    char sendPDU[PDUSIZE + 1], sendPayload[PAYLOADSIZE + 1];

    // Receive variables //
    int flag = 0, recvLength = 0, payloadLength = 0, serverAddrLength = sizeof(struct sockaddr_in6);
    uint32_t recvNumHost = 0, recvNumNetwork = 0, expectedNum = 0, highestNum = 0;
    char recvPDU[PDUSIZE + 1], recvPayload[PAYLOADSIZE + 1];
    
    // Flush variables //
    int index;

    // Set up polling //
    int pollTime = 0, pollResult = 0;
	setupPollSet();
	addToPollSet(clientSocket);

    int sentSREJ = 0;

    // File writing variables //
    FILE* file;
    file = fopen(toFileName, "w");                                          //Clear file
    if (file == NULL) {                                                     //File could not be opened
        fprintf(stderr, "Error on open of output file: %s", toFileName);
        exit(1);

    }
    fclose(file);

    file = fopen(toFileName, "a");                                          //Append to file
    if (file == NULL) {                                                     //File could not be opened
        fprintf(stderr, "Error on open of output file: %s", toFileName);
        exit(1);

    }

    int timeoutCounter = 0;
    USESTATES state = POLL;

    while (1) {
        switch (state) {
            case POLL:
                pollResult = pollCall(pollTime);

                if (timeoutCounter > 10) {
                    printf("Server unresponsive. Ending connection.\n");
                    close(clientSocket);

                    return;
                }

                if (pollResult > 0) {
                    state = READ;

                } else {
                    pollTime = 1000;            //Poll for 1 second
                    ++timeoutCounter;           //Increment counter

                }
                break;
            case READ:      //Read data and write to toFileName   
                recvLength = safeRecvfrom(clientSocket, recvPDU, PDUSIZE, 0, (struct sockaddr *) server, &serverAddrLength);
                
                if (in_cksum((unsigned short *)(recvPDU), recvLength) == 0) {
                    flag = recvPDU[6];                                          //Set flag
                
                    memcpy(&recvNumNetwork, recvPDU, 4);                        //Copy sequenceNumber
                    recvNumHost = ntohl(recvNumNetwork);                        //Convert to host

                    payloadLength = recvLength - HEADERLENGTH;
                    memcpy(recvPayload, recvPDU + HEADERLENGTH, payloadLength); //Copy payload
                    recvPayload[payloadLength] = '\0';                          //Null terminate

                    if (flag == 10) {
                        if (recvNumHost > expectedNum) {
                            state = BUFFER;

                        } else {
                            // Send final RR //
                            memset(sendPayload, '\0', sizeof(sendPayload));
                            createPDU(sendPDU, sendPayload, expectedNum, 5);
                            totalLength = HEADERLENGTH;
                            checksum = in_cksum((unsigned short *)(sendPDU), totalLength);
                            memcpy(sendPDU + 4, &checksum, 2);
                            sendtoErr(clientSocket, sendPDU, totalLength, 0, (struct sockaddr *) server, serverAddrLength);
                            
                            fclose(file);
                            return;
                        }
                    } else if (flag == 16 ||flag == 18) {
                        if (recvNumHost == expectedNum) {
                            state = SEND;

                        } else if (recvNumHost > expectedNum) {
                            state = BUFFER;

                        } else {    //Duplicate packet received
                            // Send highest RR //
                            memset(sendPayload, '\0', sizeof(sendPayload));         //Empty out sendPayload
                            createPDU(sendPDU, sendPayload, expectedNum, 5);        //Create PDU
                            totalLength = HEADERLENGTH;
                            checksum = in_cksum((unsigned short *)(sendPDU), totalLength);
                            memcpy(sendPDU + 4, &checksum, 2);
                            sendtoErr(clientSocket, sendPDU, totalLength , 0, (struct sockaddr *) server, serverAddrLength);
                            
                            state = POLL;
                        }
                    } else if (flag == 17) {
                        state = FLUSH;
                    
                    } else {
                        fprintf(stderr, "ERROR: Unknown flag received: %d\n", flag);
                        state = POLL;

                    }
                } else {
                    state = POLL;

                }
                break;
            case SEND:      //Write to disk and send RR
                // Write to disk //
                fprintf(file, "%s", recvPayload);
                fflush(file);                                           //Flush buffer

                highestNum = expectedNum;
                ++expectedNum;
                
                // Send RR //
                memset(sendPayload, '\0', sizeof(sendPayload));         //Empty out sendPayload
                createPDU(sendPDU, sendPayload, expectedNum, 5);        //Create PDU
                totalLength = HEADERLENGTH;
                checksum = in_cksum((unsigned short *)(sendPDU), totalLength);
                memcpy(sendPDU + 4, &checksum, 2);
                sendtoErr(clientSocket, sendPDU, totalLength + 1, 0, (struct sockaddr *) server, serverAddrLength);

                pollTime = 0;               //Reset poll
                timeoutCounter = 0;         //Increment counter

                state = POLL;
                break;
            case BUFFER:      //Stores out of order packets into a buffer
                // Buffer received packet //
                addToWindow(window, recvNumHost, recvPayload, windowSize, payloadLength);
                highestNum = recvNumHost;

                // Send RR //
                if (sentSREJ == 0) {
                    memset(sendPayload, '\0', sizeof(sendPayload));         //Empty out sendPayload
                    createPDU(sendPDU, sendPayload, expectedNum, 6);        //Create PDU
                    totalLength = HEADERLENGTH;
                    checksum = in_cksum((unsigned short *)(sendPDU), totalLength);
                    memcpy(sendPDU + 4, &checksum, 2);
                    sendtoErr(clientSocket, sendPDU, totalLength + 1, 0, (struct sockaddr *) server, serverAddrLength);
                    sentSREJ = 1;
                }

                state = POLL;
                break;
            case FLUSH:     //Write buffered packets to disk and send latest RR
                // Write buffered packets to disk //
                for (uint32_t i = expectedNum; i <= highestNum; ++i) {
                    index = i % windowSize;

                    if (window[index] != NULL) {
                        fprintf(file, "%s", window[index]);
                        fflush(file);            //Flush buffer
                        
                        free(window[index]);     //Free memory
                        window[index] = NULL;    //Reset element in buffer
            
                        ++expectedNum;
                    } else {
                        fprintf(stderr, "ERROR: Missing packet %d\n", i);
                        break;

                    }
                }

                // Send RR //
                memset(sendPayload, '\0', sizeof(sendPayload));         //Empty out sendPayload
                createPDU(sendPDU, sendPayload, expectedNum, 5);        //Create PDU
                totalLength = HEADERLENGTH;
                checksum = in_cksum((unsigned short *)(sendPDU), totalLength);
                memcpy(sendPDU + 4, &checksum, 2);
                sendtoErr(clientSocket, sendPDU, totalLength , 0, (struct sockaddr *) server, serverAddrLength);

                sentSREJ = 0;
                pollTime = 0;                   //Immediately return from poll
                timeoutCounter = 0;             //Reset counter

                state = POLL;
                break;
            default:
                break;
        }
    }
}