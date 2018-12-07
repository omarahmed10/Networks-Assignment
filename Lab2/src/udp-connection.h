#ifndef _UDP_CONNECTION
#define _UDP_CONNECTION

#include <stdint.h>

/* Abstract UDP implementation of the abstract Connection class*/
class UDPConnection : public Connection
{
  protected:
	int sockfd;
	struct sockaddr_in servaddr, recvaddr;

	//	unsigned int addrlen;
	// Packet recievedpacket;
//	int datalen;
//	char mesg[BUF_SIZE];

	/* called after the socket is created so that server/client implementations can
		change sockaddr_in vars */
	virtual int onSocketCreated();

  public:
	UDPConnection();
	virtual ~UDPConnection();

	/* inherited from Connection */
	virtual int connect();
	virtual void setTimeout(int secs);
	// virtual int send(Packet packet);
	// virtual int blocking_receive(char* return_buf, struct sockaddr_in servaddr);
	virtual int send(const void *message, unsigned int length, struct sockaddr_in toAddr);
	virtual int blocking_receive(char *return_buf);

	virtual struct sockaddr_in getRecvAddr();
	virtual struct sockaddr_in getServAddr();
	virtual int getSockFD();
	virtual void dumpInfo();
};

#endif
