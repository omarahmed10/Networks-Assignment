#include <stdio.h>
#include <cstring>
#include <cstdlib>
#include "stdout-server.h"
#include "../gobackn-protocol.h"
#include "../StopAndWait-protocol.h"
#include "server-udpconnection.h"

StdoutServer::StdoutServer(Protocol *p, Connection *c) {
	this->p = p;
	this->c = c;
}

bool check_if_request(char *mesg) {
	//return the type of the message ack or request from client
	if (mesg[0] == 'G' && mesg[1] == 'E' && mesg[2] == 'T')
		return true;
	return false;
}

void StdoutServer::start() {
	if (c->connect() < 0) {
		printf("Could not connect\n");
		return;
	}

	p->setConnection(c);

	while (1) {
		char *mesg = p->receiveMessage();
		struct sockaddr_in cliaddr = c->getRecvAddr();
		int port = ntohs(cliaddr.sin_port);
		stringstream ss;
		ss << port;
		string portNo = ss.str();
		string ip = inet_ntoa(cliaddr.sin_addr);
		string hash = ip;
		hash.append(" ");
		hash.append(portNo);
		if (check_if_request(mesg)) { // new request detected.

		}else{

		}
	}

	printf("server stopping\n");
}

/* Run a StdoutServer over a ServerUDPConnection using the GoBackNProtocol */
int main(int argc, char **argv) {

	/* optionally fetch the port from the cmd line, otherwise use the default */
	int port;
	if (argc <= 1) { // no args
		printf("usage: udp-server <port>\n");
		printf("using defeault port %i\n", DEFAULT_PORT);
		port = DEFAULT_PORT;
	} else {
		port = atoi(argv[1]);
	}

	ServerUDPConnection c(port);
	// GoBackNProtocol p;
	StopAndWaitProtocol p;

	StdoutServer server(&p, &c);
	server.start();
}
