/* 
 * Terminal Cmmds: cd - Enter file; cd .. - Exit file; ls - List all files; rm fileName - Remove file
 * Relevant Cmmds: ./server 0.1 5000
 * 
 */

#include "server.h"

int main(int argc, char *argv[]) {
	int portNumber = 0, serverSocket = 0;						//Server's socket descriptor
	double errorRate = 0;
	
	// Set up the UDP server //
	portNumber = checkArgs(argc, argv);
	serverSocket = udpServerSetup(portNumber);					//Generate server's socket

	errorRate = atof(argv[1]);
	sendErr_init(errorRate, DROP_ON, FLIP_ON, DEBUG_ON, RSEED_ON);

	serverControl(serverSocket, errorRate);
	close(serverSocket);
	
	return 0;
}

/* Ensure the correct # of arguments were passed. Returns a port number or terminates */
int checkArgs(int argc, char *argv[]) {
	int portNumber = 0;

	// Checks args and returns port number //
	if (argc < 2 || argc > 3) {		//Check the # of arguments
		fprintf(stderr, "Usage: %s error-rate [OPTIONAL: Port Number]\n", argv[0]);

		exit(1);
	} else if (argc == 3) {			//Set port number if specified
		portNumber = atoi(argv[2]);

	}
	return portNumber;
}

/* Accept clients and prints client messages */
void serverControl(int serverSocket, double errorRate) {
	char buffer[PDUSIZE + 1];
	
	struct sockaddr_in6 client;		
	int clientAddrLength = sizeof(client);		
	
	// Set up polling //
	int pollTime = 0, pollResult = 0;
	setupPollSet();
	addToPollSet(serverSocket);				//Add server socket to pollset				

	while (1) {
		pollResult = pollCall(pollTime);

		if (pollResult > 0) {
			safeRecvfrom(serverSocket, buffer, PDUSIZE, 0, (struct sockaddr *) &client, &clientAddrLength);

			// Check if this client is already being handled.
            if (clientExists(&client) == 0) {
				pid_t child = fork();						//Child pid

				if (child < 0) {
					perror("Fork unsuccessful\n");

				} else if (child == 0) {					//Child process
					close(serverSocket);
					sendErr_init(errorRate, DROP_ON, FLIP_ON, DEBUG_ON, RSEED_ON);
					
					int childSocket = udpServerSetup(0);	//Generate child socket
					int windowSize, bufferSize;
					char fileName[100];
					char **window;

					STATES state = SETUP;
					
					while (1) {
						switch (state) {
							case SETUP:
								serverSetUp(childSocket, buffer, &client, &windowSize, &bufferSize, fileName);
								window = initializeWindow(windowSize);
	
								state = USE;
								break;
							case USE:
								serverUse(childSocket, &client, window, windowSize, bufferSize, fileName);
								
								state = TEARDOWN;
								break;
							case TEARDOWN:
								// Free each element in the window //
								for (int i = 0; i < windowSize; i++) {
									if (window[i] != NULL) {
										free(window[i]);
									}
								}
								free(window);

								//TODO: REMOVE CLIENT!!!!!!!!!!!!!!!!!!!!!!!!
								printf("Terminating child process.\n");
								exit(0);
							default:
								break;
						}
					}
				} else {									//Parent process
					addClient(&client, child);

				}
			}
		}
	}
}