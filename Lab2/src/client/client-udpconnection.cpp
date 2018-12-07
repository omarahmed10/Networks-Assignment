#include <stdio.h>
#include <cstring>
#include <arpa/inet.h>
#include "client-udpconnection.h"

ClientUDPConnection::ClientUDPConnection() {
	printf("Client connection created\n");
}

ClientUDPConnection::ClientUDPConnection(int port, char* ip_dot_notation) {
	servaddr.sin_port   = htons(port);
	servaddr.sin_addr.s_addr = inet_addr(ip_dot_notation);
	printf("Client connection created\n");
}

ClientUDPConnection::~ClientUDPConnection() {
	printf("Client connection destroyed\n");
}
