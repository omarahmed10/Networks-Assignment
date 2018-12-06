#include <stdio.h>
#include <memory.h>
#include "common.h"
#include "udp-connection.h"

UDPConnection::UDPConnection() : servaddr{0}, cliaddr{0}
{
	sockfd = -1;
	addrlen = sizeof(struct sockaddr_in);
	servaddr.sin_family = AF_INET;

	datalen = -1;
	servaddr.sin_port = htons(DEFAULT_PORT);

	printf("Generic connection created\n");
}

int UDPConnection::connect()
{
	if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		perror("can't open datagram socket");
		return -1;
	}

	if (onSocketCreated() < 0)
	{
		perror("error in OnSocketCreated\n");
		return -1;
	}

	return 0;
}

void UDPConnection::setTimeout(int secs)
{
	timeval tval = {0};
	tval.tv_sec = secs;

	setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tval, sizeof(timeval));
}

// int UDPConnection::send(Packet packet) {
// 	sendto(sockfd, packet.data, packet.len, 0, (struct sockaddr *)&packet.servaddr, sizeof(packet.servaddr));
// 	//printf("UDPConnection send\n");
// }

// int UDPConnection::blocking_receive(char* return_buf, struct sockaddr_in addr) {
// 	int addrlen = sizeof(addr);
// 	char mesg[BUF_SIZE];
// 	int datalen = recvfrom(sockfd, mesg, BUF_SIZE, 0, (struct sockaddr *)&addr, &addrlen);
// 	recievedpacket = Packet();
// 	memcpy(&recievedpacket.data,mesg,BUF_SIZE);
// 	recievedpacket.addr = addr;
// 	memcpy(return_buf, mesg, BUF_SIZE);
// 	//printf("UDPConnection receive got %i bytes\n", datalen);
// 	return datalen;
// }

int UDPConnection::send(const void *message, unsigned int length)
{
	return sendto(sockfd, message, length, 0, (struct sockaddr *)&servaddr, sizeof(servaddr));
	//printf("UDPConnection send\n");
}

int UDPConnection::blocking_receive(char *return_buf)
{
	memset(&mesg, 0, strlen(mesg));
	datalen = recvfrom(sockfd, mesg, BUF_SIZE, 0, (struct sockaddr *)&servaddr, &addrlen);
	mesg[datalen] = '\0';
	int port = ntohs(servaddr.sin_port);
	string ip = inet_ntoa(servaddr.sin_addr);
	cout << "received " << mesg << " of len=" << datalen << " from ip=" << ip << " port=" << port << endl;
	memcpy(return_buf, mesg, BUF_SIZE);
	return datalen;
}

int UDPConnection::onSocketCreated()
{
	return 0;
}

void UDPConnection::dumpInfo()
{
	printf("sockfd: %i addrlen: ..removed..\n", sockfd);
}

UDPConnection::~UDPConnection()
{
	printf("Generic connection destroyed\n");
}