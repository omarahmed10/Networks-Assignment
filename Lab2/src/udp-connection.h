#ifndef _UDP_CONNECTION
#define _UDP_CONNECTION

#include <stdint.h>

class Packet
{
  public:
	struct sockaddr_in addr;
	unsigned int addrlen;

	u_int16_t checksum;
	u_int16_t len;
	u_int32_t seqno;

	bool is_ACK;
	char data[BUF_SIZE];

	Packet();
};

/* Abstract UDP implementation of the abstract Connection class*/
class UDPConnection : public Connection
{
  protected:
	int sockfd;
	struct sockaddr_in servaddr, cliaddr;
	unsigned int addrlen;

	// Packet recievedpacket;
	int datalen;
	char mesg[BUF_SIZE];

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
	virtual int send(const void *message, unsigned int length);
	virtual int blocking_receive(char *return_buf);

	virtual void dumpInfo();
};

#endif