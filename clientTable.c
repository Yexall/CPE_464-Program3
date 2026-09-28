#include "clientTable.h"

static ClientEntry activeClients[MAXCLIENTS];
static int numActiveClients = 0;

int clientExists(struct sockaddr_in6 *client) {
    for (int i = 0; i < numActiveClients; ++i) {
        if (memcmp(&activeClients[i].client, client, sizeof(struct sockaddr_in6)) == 0) {
            return 1;

        }
    }
    return 0;
}

void addClient(struct sockaddr_in6 *client, pid_t pid) {
    if (numActiveClients < MAXCLIENTS) {
        activeClients[numActiveClients].client = *client;
        activeClients[numActiveClients].pid = pid;
        ++numActiveClients;

    }
}

void removeClient(pid_t pid) {
    for (int i = 0; i < numActiveClients; ++i) {
        if (activeClients[i].pid == pid) {
            activeClients[i] = activeClients[numActiveClients - 1];
            --numActiveClients;
            break;

        }
    }
}