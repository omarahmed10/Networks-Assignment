#ifndef _CLIENT_UDPCONNECTION_H
#define _CLIENT_UDPCONNECTION_H

#include "../common.h"
#include "../udp-connection.h"

/* client implementation of UDPConnection */
class ClientUDPConnection : public UDPConnection {
protected:

	int cli_port = 12347;
	virtual int onSocketCreated();
public:
	ClientUDPConnection();
	ClientUDPConnection(int cliPort, int port, const char* ip_dot_notation);
	virtual ~ClientUDPConnection();
};

#endif
