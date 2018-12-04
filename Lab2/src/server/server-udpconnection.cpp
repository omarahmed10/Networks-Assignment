#include <stdio.h>
#include "server-udpconnection.h"

ServerUDPConnection::ServerUDPConnection() {
	printf("Server created\n");
}

ServerUDPConnection::ServerUDPConnection(int port) {
	servaddr.sin_port   = htons(port);
	printf("Server created\n");
}

int ServerUDPConnection::onSocketCreated() {
	if (bind(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0) {
		perror("server: can't bind local address");
		return -1;
	}

	printf("server: binding success\n");
	return 0;
}

ServerUDPConnection::~ServerUDPConnection() {
	printf("Server destroyed\n");
}
