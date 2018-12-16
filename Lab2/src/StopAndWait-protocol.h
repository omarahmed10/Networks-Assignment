//#ifndef _SIOPANDWAIT_PROTOCOL_H
//#define _SIOPANDWAIT_PROTOCOL_H
//#include "common.h"
//
///* Implementation of the Stop-And-Wait Protocol, The server sends a single datagram, and blocks until an acknowledgment from the client is
// received (or until a timeout expires). */
//class StopAndWaitProtocol: public Protocol {
//private:
////    char buffer[BUF_SIZE]; //buffer used for incoming and outgoing messages
//	int last_seq_no = 0;
//	int filesize = -1;
//	/* use the underlying connection to send a message */
//	int sendDatagram(char *p, int len, int seqno, struct sockaddr_in toAddr);
//
//	/* listen for an ACK, returns true if the socket does not timeout */
//	bool acceptAcks();
//
//	/* helper function to set timeout to 5 seconds before listening for an ACK */
//	bool listenForAck();
//
//	int sendPacket(string hash, char *line, int t, struct sockaddr_in toAddr);
//public:
//	StopAndWaitProtocol();
//
//	virtual int sendRequest(char *line, struct sockaddr_in toAddr);
//	/* send an ACK containing sequence number seqn */
//	virtual void sendAck(int ackno, struct sockaddr_in toAddr);
//
//	virtual void sendFile(string fileName, string hash,
//			struct sockaddr_in toAddr);
//
//	/* implementation of abstract Protocol::receiveMessage */
//	virtual char * receiveMessage(string fileName);
//};
//
//#endif
