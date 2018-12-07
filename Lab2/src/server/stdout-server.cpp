#include <stdio.h>
#include <cstring>
#include <cstdlib>
#include <stdlib.h>
#include <unistd.h>
#include <cstdio>
#include "stdout-server.h"
#include "../gobackn-protocol.h"
#include "../StopAndWait-protocol.h"
#include "server-udpconnection.h"

StdoutServer::StdoutServer(Protocol *p, Connection *c) {
	this->p = p;
	this->c = c;
}
map<string, map<int, Packet>> sh_mem;

int checkfile(string file) {
	ifstream myfile("data/" + file,
			ios::in | ios::binary | ios::ate);
	if (myfile.is_open()) {
		char *memblock;
		memblock = new char[MAX_DATAGRAM_SIZE];
		int size = myfile.tellg();
		myfile.close();
		return size;
	} else {
		return -1;
	}
}

void *thread_send_file(void *arg) {
	pthread_detach(pthread_self());
	ThreadData thdata = *((ThreadData *) arg);
	char cwd[BUF_SIZE];
	getcwd(cwd, sizeof(cwd));
	cout << "thread starting " << "data/" + thdata.fileName
			<< endl;
	string filePath = "data/" + thdata.fileName;
//	ifstream myfile(, ios::in | ios::binary);
	FILE* filp = fopen(filePath.c_str(), "rb");
	if (!filp) {
		cout << "Error: could not open file" << filePath << endl;
		return NULL;
	}

	char * buffer = new char[MAX_DATAGRAM_SIZE];
	int bytes = fread(buffer, 1, MAX_DATAGRAM_SIZE,
			filp);
	while (bytes > 0) {
		thdata.p->sendMessage(thdata.hash, buffer, bytes,
				thdata.addr);
		bytes = fread(buffer, 1, MAX_DATAGRAM_SIZE,
				filp);
	}
	// Done and close.
	fclose(filp);
//	if (myfile.is_open()) {
//		char *memblock;
//		memblock = new char[MAX_DATAGRAM_SIZE];
//		int size = myfile.tellg();
//		myfile.seekg(0, ios::beg);
//		while (myfile) {
//			myfile.read(memblock, MAX_DATAGRAM_SIZE);
//			int i = myfile.gcount();
//			if (i) {
//				thdata.p->sendMessage(thdata.hash, memblock,
//						strlen(memblock), thdata.addr);
//				//			sleep(10);
//			}
//		}
//		cout << "finish reading file " << endl;
//		myfile.close();
//	} else {
//		cout << "file not found" << endl;
//	}
	return NULL;
}

void StdoutServer::start() {
	if (c->connect() < 0) {
		printf("Could not connect\n");
		return;
	}
	p->setConnection(c);
	p->sh_mem = &sh_mem;
	while (1) {
		cout << "waiting for requests...." << endl;
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
		int packetNo;
		string type, file;
		iss >> packetNo;
		iss >> type;
		if (type.compare("GET") == 0) { // new request detected.
			iss >> file;
			// init shared memory for the new thread.
			map<int, Packet> thread_mem;
			sh_mem[hash] = thread_mem;

			int filesize = checkfile(file);
			if (filesize < 0) {
				cout << "File not Found" << endl;
				continue;
			}
			// sending ack for client to inform file request received.
			p->sendAck(filesize, c->getRecvAddr());

			ThreadData thdata;
			thdata.fileName.append(file);
			thdata.hash.append(hash);
			thdata.addr = cliaddr;
			thdata.p = p;
//			thread_data_mem[hash] = thdata;
			pthread_t sending_thread;
			pthread_create(&sending_thread, NULL,
					thread_send_file, (void*) &thdata);
		} else { // aCK
			int ackno = packetNo;
			cout << type << endl;
			//update the shared memory between processes
			if (sh_mem[hash].count(packetNo) > 0)
				sh_mem[hash][ackno].is_ACK = true;
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
