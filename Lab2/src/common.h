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
#include <sys/time.h>

#define DEFAULT_PORT 12346
#define BUF_SIZE 512

#define MAX_DATAGRAM_SIZE 500
#define DATAGRAM_IDENT "DATA"
#define ACK_IDENT "ACK"

using namespace std;

class Packet {
public:
	struct sockaddr_in addr;

	u_int16_t len;
	u_int32_t seqno;
	string data;

	long int send_time;
	Packet(struct sockaddr_in toAddr, u_int32_t seq_no, string messg,
			u_int16_t datalen, long int sendTime) {
		addr = toAddr;
		len = datalen;
		seqno = seq_no;
		data = messg;
		send_time = sendTime;
	}
	Packet() :
			addr { 0 } {
		seqno = 0;
		len = 0;
		send_time = 0;
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

	virtual int sendRequest(char *line, struct sockaddr_in toAddr) = 0;

	/* send an ACK containing sequence number seqn */
	virtual void sendAck(int ackno, struct sockaddr_in toAddr) = 0;

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
	u_int32_t add(u_int16_t seqno, u_int16_t mesLen, const char buff[]) {
		u_int16_t padd = mesLen % 2;
		u_int16_t word16;
		u_int32_t sum = 0;

		// cast data from char to unsigned short
		u_int16_t mes[mesLen + padd];
		for (unsigned i = 0; i < mesLen; i++) {
			mes[i] = buff[i];
		}
		if (padd)
			mes[mesLen] = 0;

		// make 16 bit words out of every two adjacent 8 bit words and
		// calculate the sum of all 16 bit words
		for (unsigned i = 0; i < mesLen + padd; i = i + 2) {
			word16 = ((mes[i] << 8) & 0xFF00) + (mes[i + 1] & 0xFF);
			sum += (u_int32_t) word16;
		}
		// the seq number and the length of the data
		sum += seqno + mesLen;

		return sum;
	}

	u_int16_t carryComp(u_int32_t sum) {
		// keep only the last 16 bits of the 32 bit calculated sum and add the carries
		while (sum >> 16)
			sum = (sum & 0xFFFF) + (sum >> 16);
		// Take the one's complement of sum
		sum = ~sum;

		return sum;
	}

	u_int16_t getChecksum(u_int16_t seqno, u_int16_t mesLen,
			const char buff[]) {
		u_int32_t sum = add(seqno, mesLen, buff);

		return carryComp(sum);
	}

	bool isValid(u_int16_t seqno, u_int16_t mesLen, const char buff[],
			u_int16_t checksum) {
		u_int32_t sum = add(seqno, mesLen, buff);

		sum += checksum;

		return !carryComp(sum);
	}

	long int nowTime() {
		struct timeval tp;
		gettimeofday(&tp, NULL);
		return tp.tv_sec /*+ (1.0 / 1000000) * tp.tv_usec*/;
	}
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
