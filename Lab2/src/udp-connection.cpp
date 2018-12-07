#include <stdio.h>
#include <memory.h>
#include "common.h"
#include "udp-connection.h"

UDPConnection::UDPConnection() :
		servaddr { 0 }, recvaddr { 0 } {
	sockfd = -1;
//	addrlen = sizeof(struct sockaddr_in);
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(DEFAULT_PORT);
//	datalen = -1;

	printf("Generic connection created\n");
}

int UDPConnection::connect() {
	if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		perror("can't open datagram socket");
		return -1;
	}

	if (onSocketCreated() < 0) {
		perror("error in OnSocketCreated\n");
		return -1;
	}

	return 0;
}

void UDPConnection::setTimeout(int secs) {
	timeval tval = { 0 };
	tval.tv_sec = secs;

	setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tval,
			sizeof(timeval));
}

int UDPConnection::send(const void *message, unsigned int length,
		struct sockaddr_in toAddr) {
	int port = ntohs(toAddr.sin_port);
	string ip = inet_ntoa(toAddr.sin_addr);
//	cout << "sending " << (char *)message << " to " << ip << " port "
//			<< port << endl;
	return sendto(sockfd, message, length, 0,
			(struct sockaddr *) &toAddr, sizeof(toAddr));
}

int UDPConnection::blocking_receive(char *return_buf) {
	char mesg[BUF_SIZE];
	socklen_t len = sizeof(struct sockaddr);
	memset(&recvaddr, 0, sizeof(recvaddr));
	int datalen = recvfrom(sockfd, mesg, BUF_SIZE, 0,
			(struct sockaddr *) &recvaddr, &len);
	mesg[datalen] = '\0';

	memcpy(return_buf, mesg, BUF_SIZE);
	if (datalen < 0) {
		cout << "nothing received" << endl;
		return datalen;
	}

	int port = ntohs(recvaddr.sin_port);
	string ip = inet_ntoa(recvaddr.sin_addr);

	cout << "received of len=" << datalen << " from ip=" << ip
			<< " port=" << port << endl;

	return datalen;
}

int UDPConnection::onSocketCreated() {
	return 0;
}

struct sockaddr_in UDPConnection::getRecvAddr() {
	return recvaddr;
}

struct sockaddr_in UDPConnection::getServAddr() {
	return servaddr;
}

int UDPConnection::getSockFD() {
	return sockfd;
}

void UDPConnection::dumpInfo() {
	printf("sockfd: %i addrlen: ..removed..\n", sockfd);
}

UDPConnection::~UDPConnection() {
	printf("Generic connection destroyed\n");
}
