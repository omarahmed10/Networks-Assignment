#ifndef _COMMON_H
#define _COMMON_H
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <iostream>
#include <fstream>
#include <sstream>

#define DEFAULT_PORT 12346
#define BUF_SIZE 512

#define MAX_DATAGRAM_SIZE 512
#define DATAGRAM_IDENT "DATA"
#define ACK_IDENT "ACK"

using namespace std;
/* Abstract connection that defines the necessary socket i/o functions.
	Implementations may decide which transport layer protocol to use */
class Connection {
public:
	/* start the connection, create the socket, etc.*/
	virtual int connect() = 0;

	/* set the recv timeout on the socket */
	virtual void setTimeout(int secs) = 0;

	/* send array of length 'length starting at pointer message */
	virtual int send(const void *message, unsigned int length) = 0;

	/* block until a message can be found or the timeout is reached
		message is copyed to the memory pointed to by return_buf
		-1 should be returned if an error occurs */
	virtual int blocking_receive(char* return_buf) = 0;
};


/* Abstract class that defines a protocol wrapping a connection c.
	Use the sendMessage & receiveMessage to interact with the connection
	object using the implemented procotol */
class Protocol {
protected:
	Connection *c;
public:

	/* set connection to interact with */
	void setConnection(Connection *c) {
		this->c = c;
	};

	/* send a message of size t starting at the memory address pointed to by line
		through the implememted protocol */
	virtual int sendMessage(char* line, unsigned int t, bool recvACK) = 0;

	/* receive a message through the implemented protocol */
	virtual char* receiveMessage(bool sendACK) = 0;
};

#endif