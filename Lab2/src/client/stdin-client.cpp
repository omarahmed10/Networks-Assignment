#include <stdio.h>
#include <cstring>
#include <cstdlib>
#include "stdin-client.h"
#include "../gobackn-protocol.h"
#include "../StopAndWait-protocol.h"
#include "client-udpconnection.h"

StdinClient::StdinClient(Protocol *p, Connection *c) {
	this->p = p;
	this->c = c;
}

void StdinClient::start() {

	if (c->connect() < 0) {
		printf("Could not connect\n");
		return;
	}

	p->setConnection(c);

	char filerequest[] = "GET image1.png";
	p->sendMessage(filerequest, strlen(filerequest),
			c->getServAddr());

	const char *mesg;
	while ((mesg = p->receiveMessage()) != NULL) {
	}

}

/* Run a StdinClient over a ClientUDPConnection using the GoBackNProtocol */
int main(int argc, char **argv) {

	/* fetch the server_ip in dot dotation form from the cmd line, port is optional */
	int port;
	if (argc <= 1) { // no args
		printf("usage: udp-client <server_ip> <port>\n");
		exit(-1);
	} else if (argc <= 2) { // got an ip, but no port
		port = DEFAULT_PORT;
	} else {
		port = atoi(argv[2]);
	}

	ClientUDPConnection c(port, argv[1]);
	// GoBackNProtocol p;
	StopAndWaitProtocol p;

	StdinClient client(&p, &c);
	client.start();
}
