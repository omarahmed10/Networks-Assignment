#include <stdio.h>
#include <cstring>
#include <cstdlib>
#include "stdin-client.h"
#include "../gobackn-protocol.h"
#include "../StopAndWait-protocol.h"
#include "../SR-protocol.h"
#include "client-udpconnection.h"

StdinClient::StdinClient(Protocol *p, Connection *c) {
	this->p = p;
	this->c = c;
}

string filename;
void StdinClient::start() {

	if (c->connect() < 0) {
		printf("Could not connect\n");
		return;
	}

	p->setConnection(c);
	filename.append(" ");
	char filerequest[MAX_DATAGRAM_SIZE] = "GET ";
	strcat(filerequest, filename.c_str());

	int filesize = p->sendRequest(filerequest, c->getServAddr());

	if (filesize > 1) {
		p->receiveMessage(filename);
		cout << "Client received all file" << endl;
	}else{
		cout << "file not found" << endl;
	}
}
/* Run a StdinClient over a ClientUDPConnection using the GoBackNProtocol */
int main(int argc, char **argv) {

	//client file
	ifstream infile("client.in");
	string line;
	getline(infile, line);
	string ip = line;
	getline(infile, line);
	int serv_port = atoi(line.c_str());
	getline(infile, line);
	int client_port = atoi(line.c_str());
	getline(infile, line);
	filename.append(line);
	getline(infile, line);
	int window = atoi(line.c_str());
	infile.close();
	//end of client file

	ClientUDPConnection c(client_port, serv_port, ip.c_str());
	// GoBackNProtocol p;
	// StopAndWaitProtocol p;
	GoBackNProtocol p;
	p.windowsize = window;

	StdinClient client(&p, &c);
	client.start();
}
