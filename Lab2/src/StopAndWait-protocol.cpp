#include <stdio.h>
#include <cstdlib>
#include <cstring>
#include <errno.h>
#include <exception>
#include <sys/time.h>
#include "StopAndWait-protocol.h"

#define MIN(a, b) \
    ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a < _b ? _a : _b; })

int filesize = -1;

StopAndWaitProtocol::StopAndWaitProtocol() {
	c = NULL;
}

int StopAndWaitProtocol::sendDatagram(char *p, int seqno,
		struct sockaddr_in toAddr) {
	char *pTemp;
	pTemp = new char[BUF_SIZE];
	sprintf(pTemp, "%d %s", seqno, p);
	return c->send(pTemp, MIN(BUF_SIZE, strlen(pTemp)), toAddr);
}

int StopAndWaitProtocol::sendRequest(char *line,
		struct sockaddr_in toAddr) {
	int sendsize = -1;
	for (;;) {
		sendsize = sendDatagram(line, -1, toAddr);
		if (acceptAcks()) {
			break;
		}
	}
	return sendsize;
}

int StopAndWaitProtocol::sendMessage(string hash, char *line,
		int t, struct sockaddr_in toAddr) {
	int sendsize = -1;
	(*sh_mem)[hash][last_seq_no] = Packet();
	(*sh_mem)[hash][last_seq_no].data = line;
	(*sh_mem)[hash][last_seq_no].seqno = last_seq_no;
	for (;;) {
		struct timeval tp;
		gettimeofday(&tp, NULL);
		long int s = tp.tv_sec;
		sendsize = sendDatagram(line, last_seq_no, toAddr);
		bool acked = false;
		while (!acked) {
			struct timeval tp;
			gettimeofday(&tp, NULL);
			long int nows = tp.tv_sec;
			if ((*sh_mem)[hash][last_seq_no].is_ACK) { // acked
				last_seq_no++;
				acked = true;
				break;
			}
			if (nows - s >= 10) {
				break;
			}
		}
		if (acked) {
			break;
		}
	}
	return sendsize;
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
	istringstream iss(buf); // 0000 ACK   /// 000 GET asdfasdf
	iss >> filesize;
	return true;
}

void StopAndWaitProtocol::sendAck(int ackno,
		struct sockaddr_in toAddr) {
	char *pTemp;
	pTemp = new char[BUF_SIZE];
	sprintf(pTemp, "%d %s", ackno, ACK_IDENT);
	c->send(pTemp, strlen(pTemp), toAddr);
}

char *StopAndWaitProtocol::receiveMessage(string fileName) {
	int datagram_seqn;
	char payload;

	if (c == NULL) {
		throw std::exception();
	}
	cout << "blocking for receive file of size " << filesize
			<< endl;
	char *buf;
	buf = new char[BUF_SIZE];
	ofstream myfile("recvData/" + fileName,
			ios::out | ios::binary);
	if (!myfile.is_open()) {
		return NULL;
	}
	while (c->blocking_receive(buf) != -1) {
		istringstream ss(buf);
		int ackno;
		ss >> ackno;
		string mes = buf;
		string data = mes.substr(mes.find(' ') + 1);
		myfile << data;
		sendAck(ackno, c->getRecvAddr());
		filesize -= data.size();
		cout << "rema " << filesize << endl;
		if (filesize <= 0)
			break;
	}
	myfile.close();
	return buf;
}
