#include <stdio.h>
#include <cstdlib>
#include <cstring>
#include <errno.h>
#include <exception>
#include "StopAndWait-protocol.h"

#define MIN(a, b) \
    ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a < _b ? _a : _b; })

StopAndWaitProtocol::StopAndWaitProtocol() {
	c = NULL;
}

int StopAndWaitProtocol::sendDatagram(char *p,
		struct sockaddr_in toAddr) {
	char *pTemp;
	pTemp = new char[BUF_SIZE];
	sprintf(pTemp, "%d %s", last_seq_no++, p);
	return c->send(pTemp, MIN(BUF_SIZE, strlen(pTemp)), toAddr);
}

int StopAndWaitProtocol::sendMessage(char *line, int t,
		struct sockaddr_in toAddr) {
	if (c == NULL) {
		throw std::exception();
	}
	int total_byte = 0;
	int current_byte = 0;
	/* loop while we have more to send or are still waiting
	 on outstanding ACKs */
	while (t > 0) {
		/* send as many as we can*/
		line += current_byte;
		int sendlen = sendDatagram(line, toAddr);
		current_byte = 0;
		/* listen */
		if (acceptAcks()) {
			current_byte = sendlen; // advance the line pointer when ack received.
			t -= sendlen;
			total_byte += sendlen;
		} else {
			std::cout << "NACK" << std::endl;
		}
	}

	printf("ALL BYTES SENT for line %s\n", line);
	return total_byte;
}

bool StopAndWaitProtocol::acceptAcks() {
	bool didNotTimeout;

	c->setTimeout(10);
	didNotTimeout = listenForAck();
	c->setTimeout(0);

	return didNotTimeout;
}

bool StopAndWaitProtocol::listenForAck() {
	int mesg_seqn;

	/* listen for an incoming datagram, stop if it times out */
	cout << "waiting for ACK...." << endl;
	char buf[BUF_SIZE];
	if (c->blocking_receive(buf) == -1) {
		printf("TIMEOUT\n");
		return false;
	}
	int ack_seqn;
	sscanf(buf, ACK_IDENT " %d", &ack_seqn);
	cout << "ACK recv for " << ack_seqn << endl;
	return true;
}

void StopAndWaitProtocol::sendAck(int ackno,
		struct sockaddr_in toAddr) {
	char *pTemp;
	pTemp = new char[BUF_SIZE];
	sprintf(pTemp, ACK_IDENT " %d", ackno);
	c->send(pTemp, strlen(pTemp), toAddr);
}

char *StopAndWaitProtocol::receiveMessage() {
	int datagram_seqn;
	char payload;

	if (c == NULL) {
		throw std::exception();
	}
	std::cout << "blocking for receive" << std::endl;
	char *buf;
	buf = new char[BUF_SIZE];
	Packet *p = new Packet();
	while (c->blocking_receive(buf) != -1) {
		int ackno = 10;
		p->data = buf;
		p->is_ACK = false;
		cout << "sending ACK for " << ackno << endl;
		sendAck(ackno, c->getRecvAddr());
	}
	return buf;
}
