#ifndef _GOBACKN_PROTOCOL_H
#define _GOBACKN_PROTOCOL_H

#include "common.h"
#include <vector>
#include <queue>

class GoBackNProtocol: public Protocol {
private:
//    char buffer[BUF_SIZE]; //buffer used for incoming and outgoing messages
	int next_seq_num = 0, filesize = -1, send_base = 0,
			recv_base = 0,  packNo = 0, losscnt =
					0;
	int state = 1; // slow start >> 1, congestion avoidance >> 2, fast recovery >> 3
	float ssthresh = 0;
	long int timeout = 1;
	queue<int> threeACKpck;
	vector<int> ACKs;
	map<int, Packet> allPackets;
	vector<Packet> window;

	void readCongList();
	bool checktimeout(map<int, bool> ack_mem);
	/* listen for an ACK, returns true if the socket does not timeout */
	bool acceptAcks();

	/* helper function to set timeout to 5 seconds before listening for an ACK */
	bool listenForAck();


public:
	float windowsize = 1;
	GoBackNProtocol();

	virtual int sendRequest(char *line,
			struct sockaddr_in toAddr);
	virtual void sendAck(int ackno, struct sockaddr_in toAddr);

	virtual void sendFile(string fileName, void* data);
	virtual void addACK(int seq_no);
	virtual int sendDatagram(const char *p, int len, int seqno,
			struct sockaddr_in toAddr);
	virtual char * receiveMessage(string fileName);

};

#endif
