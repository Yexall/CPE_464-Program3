#ifndef __CLIENTTABLE_H__
#define __CLIENTTABLE_H__

// ---------- Includes ---------- //
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>

// ---------- Structs ---------- //
typedef struct {
    struct sockaddr_in6 client;
    pid_t pid;
} ClientEntry;

// ---------- Function Prototypes ---------- //
int clientExists(struct sockaddr_in6 *client);    // Returns 1 if the client exists, 0 otherwise.
void addClient(struct sockaddr_in6 *client, pid_t pid);   // Adds a client entry.
void removeClient(pid_t pid);   // Removes a client entry by PID.

// ---------- Macros ---------- //
#define MAXCLIENTS 100

#endif

