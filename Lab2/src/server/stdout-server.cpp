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

void StdoutServer::start() {
	if (c->connect() < 0) {
		printf("Could not connect\n");
		return;
	}

	p->setConnection(c);

	char *mesg;
	while ((mesg = p->receiveMessage()) != NULL) {
		fputs(mesg,stdout);
		printf("\n");
	}

	printf("server stopping\n");

}


/* Run a StdoutServer over a ServerUDPConnection using the GoBackNProtocol */
int main(int argc,char **argv) {

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