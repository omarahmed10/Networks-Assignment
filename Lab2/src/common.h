#ifndef _COMMON_H
#define _COMMON_H
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <map>
#include <string>

#define DEFAULT_PORT 12346
#define BUF_SIZE 512

#define MAX_DATAGRAM_SIZE 500
#define DATAGRAM_IDENT "DATA"
#define ACK_IDENT "ACK"

using namespace std;

class Packet {
public:
	struct sockaddr_in addr;

	u_int16_t checksum;
	u_int16_t len;
	u_int32_t seqno;

	bool is_ACK;
	string data;

	Packet() :
			addr { 0 } {
		seqno = 0;
		len = 0;
		checksum = 0;
		is_ACK = false;
		data = "";
	}

	bool operator==(const Packet& p) const {
		return seqno == p.seqno;
	}
};

//map<string, ThreadData> thread_data_mem;

/* Abstract connection that defines the necessary socket i/o functions.
 Implementations may decide which transport layer protocol to use */
class Connection {
public:
	/* start the connection, create the socket, etc.*/
	virtual int connect() = 0;

	/* set the recv timeout on the socket */
	virtual void setTimeout(int secs) = 0;

	/* send array of length 'length starting at pointer message */
	virtual int send(const void *message, unsigned int length,
			struct sockaddr_in toAddr) = 0;

	/* block until a message can be found or the timeout is reached
	 message is copyed to the memory pointed to by return_buf
	 -1 should be returned if an error occurs */
	virtual int blocking_receive(char* return_buf) = 0;

	virtual struct sockaddr_in getRecvAddr() = 0;
	virtual struct sockaddr_in getServAddr() = 0;
	virtual int getSockFD() = 0;

	virtual ~Connection() {
	}
	;
};

/* Abstract class that defines a protocol wrapping a connection c.
 Use the sendMessage & receiveMessage to interact with the connection
 object using the implemented procotol */
class Protocol {
protected:
	Connection *c;

public:

	float seed = 1.0, error = 0.4;
//	map<string, map<int, Packet>>* sh_mem;
	/* set connection to interact with */
	void setConnection(Connection *c) {
		this->c = c;
	}
	;

	virtual int sendRequest(char *line,
			struct sockaddr_in toAddr) = 0;

	/* send an ACK containing sequence number seqn */
	virtual void sendAck(int ackno,
			struct sockaddr_in toAddr) = 0;

	/* send a message of size t starting at the memory address pointed to by line
	 through the implememted protocol */
	virtual void sendFile(string fileName, void* arg) = 0;
//	virtual int sendPacket(string hash, char* line, int t,
//			struct sockaddr_in toAddr) = 0;

	virtual char* receiveMessage(string fileName) = 0;

	virtual void addACK(int seq_no) = 0;

	virtual int sendDatagram(const char *p, int len, int seqno,
			struct sockaddr_in toAddr) = 0;

	virtual ~Protocol() {
	}
	;

};

template<typename T>
string NumberToString(T Number) {
	ostringstream ss;
	ss << Number;
	return ss.str();
}
struct ThreadClientData {
	string fileName, hash;
	struct sockaddr_in addr;
	Protocol *p;
//	map<int, Packet> thmem;
	map<string, map<int, bool>>* sh_mem;
};
struct ThreadPacketData {
	string hash;
	string data;
	int datalen, seq_no;
	struct sockaddr_in addr;
	Protocol *p;
	map<string, map<int, bool>>* sh_mem;
//	map<int, Packet> thmem;
};
#endif
