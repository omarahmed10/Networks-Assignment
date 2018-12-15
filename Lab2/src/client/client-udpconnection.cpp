#include <stdio.h>
#include <cstring>
#include <arpa/inet.h>
#include "client-udpconnection.h"

ClientUDPConnection::ClientUDPConnection() {
	printf("Client connection created\n");
}

ClientUDPConnection::ClientUDPConnection(int cliPort, int port,
		const char* ip_dot_notation) {
	cli_port = cliPort;
	servaddr.sin_port = htons(port);
	servaddr.sin_addr.s_addr = inet_addr(ip_dot_notation);
	printf("Client connection created\n");
}

int ClientUDPConnection::onSocketCreated() {
	struct sockaddr_in cliaddr;
	cliaddr.sin_family = AF_INET;
	cliaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	cliaddr.sin_port = htons(cli_port);
	if (bind(sockfd, (struct sockaddr *) &cliaddr,
			sizeof(cliaddr)) < 0) {
		perror("client: can't bind port");
		return -1;
	}

	printf("client: binding success\n");
	return 0;
}

ClientUDPConnection::~ClientUDPConnection() {
	printf("Client connection destroyed\n");
}
