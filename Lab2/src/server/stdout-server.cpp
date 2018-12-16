#include <stdio.h>
#include <cstring>
#include <cstdlib>
#include <stdlib.h>
#include <unistd.h>
#include <cstdio>
#include "stdout-server.h"
#include "../gobackn-protocol.h"
#include "../StopAndWait-protocol.h"
#include "../SR-protocol.h"
#include "server-udpconnection.h"

StdoutServer::StdoutServer(Protocol *p, Connection *c) {
	this->p = p;
	this->c = c;
}
map<string, map<int, bool>> sh_mem;
int window;

int checkfile(string file) {
	ifstream myfile("data/" + file,
			ios::in | ios::binary | ios::ate);
	if (myfile.is_open()) {
		int size = myfile.tellg();
		myfile.close();
		return size;
	} else {
		return -1;
	}
}

void *thread_send_file(void *arg) {
	pthread_detach(pthread_self());
	ThreadClientData thdata = *((ThreadClientData *) arg);
	char cwd[BUF_SIZE];
	getcwd(cwd, sizeof(cwd));
	cout << "thread starting " << "data/" + thdata.fileName
			<< endl;
	string filePath = "data/" + thdata.fileName;
	thdata.p->sendFile(filePath, arg);
	return NULL;
}

void StdoutServer::start() {
	if (c->connect() < 0) {
		printf("Could not connect\n");
		return;
	}
	p->setConnection(c);
	int clients = 0;
//	p->sh_mem = &sh_mem;
	while (1) {
		char mesg[BUF_SIZE];
		c->blocking_receive(mesg);

		struct sockaddr_in cliaddr = c->getRecvAddr();
		int port = ntohs(cliaddr.sin_port);
		stringstream ss;
		ss << port;
		string portNo = ss.str();
		string ip = inet_ntoa(cliaddr.sin_addr);
		string hash = ip;
		hash.append(" ");
		hash.append(portNo);
		string request = mesg;
		istringstream iss(request); // 0000 ACK   /// 000 GET asdfasdf
		int packetNo, checksum;
		string type, file;
		iss >> packetNo >> checksum;
		iss >> type;
		if (type.compare("GET") == 0) { // new request detected.
			iss >> file;
			cout << "req for " << request << endl;

			// init shared memory for the new thread.
			map<int, bool> thread_mem;
			sh_mem[hash] = thread_mem;

			int filesize = checkfile(file);
			// sending ack for client to inform file request received.
			p->sendAck(filesize, c->getRecvAddr());
			if (filesize < 0) {
				cout << "File not Found" << endl;
				continue;
			}

			ThreadClientData thdata;
//			thdata.thmem = thread_mem;
			thdata.sh_mem = &sh_mem;
			thdata.fileName.append(file);
			thdata.hash.append(hash);
			thdata.addr = cliaddr;
			GoBackNProtocol *sr = new GoBackNProtocol();
			sr->seed = p->seed;
			sr->error = p->error;
			sr->setConnection(c);
			sr->windowsize = window;
			thdata.p = sr;
			pthread_t sending_thread;
			pthread_create(&sending_thread, NULL,
					thread_send_file, (void*) &thdata);
		} else { // aCK
			int ackno = packetNo;
			//update the shared memory between processes
			if (sh_mem[hash].count(packetNo) > 0) {
				sh_mem[hash][ackno]/*.is_ACK */= true;
			}
		}
	}
	printf("server stopping\n");
}

/* Run a StdoutServer over a ServerUDPConnection using the GoBackNProtocol */
int main(int argc, char **argv) {

	//server file
	ifstream infile("server.in");
	string line;
	getline(infile, line);
	int serv_port = atoi(line.c_str());
	getline(infile, line);
	window = atoi(line.c_str());
	getline(infile, line);
	float seed = atof(line.c_str());
	getline(infile, line);
	float error = atof(line.c_str());
	//end of server file

	ServerUDPConnection c(serv_port);
	// GoBackNProtocol p;
//	StopAndWaitProtocol p;
	GoBackNProtocol p;
	p.seed = seed;
	p.error = error;
	StdoutServer server(&p, &c);
	server.start();
}
