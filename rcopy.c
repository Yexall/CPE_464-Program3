/* 
 * Terminal Cmmds: cd - Enter file; cd .. - Exit file; ls - List all files; rm fileName - Remove file
 * Relevant Cmmds: ./rcopy fromFile.txt toFile.txt 5 100 0.5 localhost 5000
 * 
 */

#include "rcopy.h"

int main(int argc, char *argv[]) {
	int portNumber = 0;				//Server's port number
	int clientSocket = 0;         	//Client's socket descriptor
	double errorRate = 0;
	struct sockaddr_in6 server;		//Supports 4 and 6 but requires IPv6 struct
	
	// Set up the UDP client //
	portNumber = checkArgs(argc, argv);
	clientSocket = setupUdpClientToServer(&server, argv[6], portNumber);	//Generate client's socket

	errorRate = atof(argv[5]);
	sendErr_init(errorRate, DROP_ON, FLIP_ON, DEBUG_ON, RSEED_ON);

	clientControl(clientSocket, &server, atoi(argv[3]), atoi(argv[4]), argv[1], argv[2]);
	close(clientSocket);

	return 0;
}

/* Ensure the format and the correct # of argumenets were passed. Returns a port number or terminates */
int checkArgs(int argc, char *argv[]) {
	int portNumber = 0;

	if (argc != 8) {											//Check the # of arguements
		fprintf(stderr, "Usage: %s from-filename to-filename window-size buffer-size error-rate remote-machine remote-port. Terminating client.\n", argv[0]);
		
		exit(1);
	}
	if ((strlen(argv[1]) > 100) || (strlen(argv[2]) > 100)) {	//Check filename length
		fprintf(stderr, "Error: File names must be less than 100 characters. Terminating client.\n");
		
		exit(1);
	}
	if (atof(argv[5]) < 0 || atof(argv[5]) > 1) {				//Check if error-rate is between 0 - 100%
		fprintf(stderr, "Error: Error rate must be a floating point number from 0 - 1. Terminating client.\n");
		
		exit(1);
	}
	portNumber = atoi(argv[7]);

	return portNumber;
}

void clientControl(int clientSocket, struct sockaddr_in6 *server, int windowSize, int bufferSize, char *fromFileName, char *toFileName) {
	char **window;
	STATES state = SETUP;
	
	switch (state) {
		case SETUP:
			rcopySetUp(clientSocket, server, windowSize, bufferSize, fromFileName);
			window = initializeWindow(windowSize);

			state = USE;

		case USE:
			rcopyUse(clientSocket, server, window, windowSize, toFileName);

			state = TEARDOWN;
		case TEARDOWN:
			// Free each element in the window //
			for (int i = 0; i < windowSize; i++) {
				if (window[i] != NULL) {
					free(window[i]);
				}
			}
			free(window);

			printf("Terminating client.\n");
			exit(0);
		default:
			break;
	}
}

/* Reads input from stdin: Ensure the input length < buffer size and null terminates the string */
int readFromStdin(char *buffer) {
	char inputChar = 0;
	int bufferIndex = 0;        
	
	// Important you don't input more characters than you have space 
	buffer[0] = '\0';
	while (bufferIndex < (PAYLOADSIZE - 1) && inputChar != '\n') {
		inputChar = getchar();
		
		if (inputChar != '\n') {
			buffer[bufferIndex] = inputChar;
			bufferIndex++;
		}
	}
	
	// Null terminate the string //
	buffer[bufferIndex] = '\0';
	bufferIndex++;
	
	return bufferIndex;
}