#pragma once
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <errno.h>
#define LIBSSH_STATIC 1
#include <libssh/libssh.h>
#include "communication.h"


// Connect to server
int prepare_socket(char* IP, int PORT);

// Verify known hosts
int verify_knownhost(ssh_session session);
