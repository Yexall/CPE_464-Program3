#include "PDU.h"

void createPDU(char *PDU, char *payload, uint32_t packetSequence, int flag) {
    int payloadLength = strlen(payload);
    uint16_t checksum = 0;
    uint32_t netOrder_packetSeq = htonl(packetSequence);

    memcpy(PDU, &netOrder_packetSeq, 4);                            //32 bit Sequence number (4 bytes)
    memcpy(PDU + 4, &checksum, 2);                                  //Checksum number (2 bytes)
    memcpy(PDU + 6, &flag, 1);                                      //Flag (1 byte)
    memcpy(PDU + 7, payload, payloadLength);                        //Payload
    PDU[7 + payloadLength] = '\0';                                  //Null terminate
}