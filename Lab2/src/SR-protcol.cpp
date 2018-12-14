/*
 * SR-protcol.cpp
 *
 *  Created on: Dec 8, 2018
 *      Author: omar
 */
#include <stdio.h>
#include <cstdlib>
#include <cstring>
#include <errno.h>
#include <exception>
#include <sys/time.h>
#include <algorithm>
#include "SR-protocol.h"

#define MIN(a, b) \
    ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a < _b ? _a : _b; })

pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mux = PTHREAD_MUTEX_INITIALIZER;

SRProtocol::SRProtocol() {
	c = NULL;
	if (pthread_mutex_init(&lock, NULL) != 0) {
		printf("\n mutex init has failed\n");
		exit(-1);
	}
}

int SRProtocol::sendDatagram(const char *p, int len, int seqno,
		struct sockaddr_in toAddr) {
	char *pTemp;
	pTemp = new char[BUF_SIZE];
	stringstream ss;
	ss << seqno << " ";
	sprintf(pTemp, "%d %s", seqno, p);
	pthread_mutex_lock(&lock);
	cout << "\033[1;34mseding packet no" << seqno << " size="
			<< (ss.str().size() + len) << "\033[0m\n" /*<< p
			 << "\n================"*/<< endl;
	int sendsize = c->send(pTemp,
			MIN(BUF_SIZE, ss.str().size() + len), toAddr);
	pthread_mutex_unlock(&lock);
	return sendsize;
}

int SRProtocol::sendRequest(char *line,
		struct sockaddr_in toAddr) {
	int sendsize = -1;
	for (;;) {
		sendsize = sendDatagram(line, strlen(line), -1, toAddr);
		if (acceptAcks()) {
			break;
		}
	}
	return sendsize;
}

void *sendPacket(void *arg) {
	pthread_detach(pthread_self());
	ThreadPacketData thdata = *((ThreadPacketData *) arg);
	cout << "start thread " << thdata.seq_no << endl;
	pthread_cond_signal(&cond);
	int sendsize = -1;
	(*thdata.sh_mem)[thdata.hash][thdata.seq_no] = Packet();
	(*thdata.sh_mem)[thdata.hash][thdata.seq_no].data.append(
			thdata.data);
	(*thdata.sh_mem)[thdata.hash][thdata.seq_no].seqno =
			thdata.seq_no;
	for (int i = 0; i < 10; i++) {
		struct timeval tp;
		gettimeofday(&tp, NULL);
		long int s = tp.tv_sec;
		float ran = (float) (rand() % 100) / 100;
		if (ran <= 0.4) {
//			cout << "packet no" << thdata.seq_no << " is lost"
//					<< endl;
		} else {
			sendsize = thdata.p->sendDatagram(
					thdata.data.c_str(), thdata.datalen,
					thdata.seq_no, thdata.addr);
		}
		bool acked = false;
		while (!acked) {
			struct timeval tp;
			gettimeofday(&tp, NULL);
			long int nows = tp.tv_sec;
			if ((*thdata.sh_mem)[thdata.hash][thdata.seq_no].is_ACK) { // acked
				thdata.p->addACK(thdata.seq_no);
				acked = true;
				break;
			}
			if (nows - s >= 2) {
				break;
			}
		}
		if (acked) {
			break;
		}
	}
	return NULL;
}

void SRProtocol::sendFile(string fileName, void* arg) {
	ThreadClientData carg = *((ThreadClientData *) arg);
	ifstream myfile(fileName, ios::in | ios::binary | ios::ate);
	if (myfile.is_open()) {
		char *memblock;
		memblock = new char[MAX_DATAGRAM_SIZE];
		int size = myfile.tellg();
		myfile.seekg(0, ios::beg);
		int totalSize = 0;
		srand(7.0);
		while (myfile) {
			pthread_mutex_lock(&lock);
			if (next_seq_num - send_base >= windowsize) { // window is full cannot send a new packet.
				pthread_mutex_unlock(&lock);
				continue;
			}
			pthread_mutex_unlock(&lock);
			memset(memblock, 0, MAX_DATAGRAM_SIZE);
			myfile.read(memblock, MAX_DATAGRAM_SIZE);
			int i = myfile.gcount();
			totalSize += i;
			if (i) {
				ThreadPacketData ptthdata;
				ptthdata.data.append(memblock);
				ptthdata.hash.append(carg.hash);
				ptthdata.addr = carg.addr;
				ptthdata.sh_mem = carg.sh_mem;
//				ptthdata.thmem = carg.thmem;
				ptthdata.datalen = i;
				ptthdata.seq_no = next_seq_num;
				ptthdata.p = carg.p;
				pthread_t sending_thread;
				pthread_create(&sending_thread, NULL, sendPacket,
						(void*) &ptthdata);
				pthread_cond_wait(&cond, &mux);
				next_seq_num++;
			}
		}
		cout << "finish reading file " << totalSize << endl;
		myfile.close();
	} else {
		cout << "file not found" << endl;
	}
}

void SRProtocol::addACK(int seq_no) {
	pthread_mutex_lock(&lock);
	ACKs.push_back(seq_no);
	sort(ACKs.begin(), ACKs.end());
	if (seq_no == send_base) {
		bool found = false;
		for (unsigned i = 0; i < ACKs.size(); i++) {
			if (ACKs[i] - send_base >= 1) {
				ACKs.erase(ACKs.begin(), ACKs.begin() + i);
				found = true;
				break;
			}
			send_base++;
		}
		if (!found) {
			send_base = ACKs.back() + 1;
			ACKs.clear();
		}
	}
	pthread_mutex_unlock(&lock);
}

bool SRProtocol::acceptAcks() {
	bool didNotTimeout;

	c->setTimeout(5);
	didNotTimeout = listenForAck();
	c->setTimeout(0);

	return didNotTimeout;
}

bool SRProtocol::listenForAck() {
	int mesg_seqn;

	/* listen for an incoming datagram, stop if it times out */
	cout << "waiting for ACK...." << endl;
	char buf[BUF_SIZE];
	if (c->blocking_receive(buf) == -1) {
		printf("TIMEOUT\n");
		return false;
	}
	istringstream iss(buf); // 0000 ACK   /// 000 GET asdfasdf
	iss >> filesize;
	return true;
}

void SRProtocol::sendAck(int ackno, struct sockaddr_in toAddr) {
	char *pTemp;
	pTemp = new char[BUF_SIZE];
	sprintf(pTemp, "%d %s", ackno, ACK_IDENT);
	c->send(pTemp, strlen(pTemp), toAddr);
}

bool sortA(const Packet &a, const Packet &b) {
	return a.seqno < b.seqno;
}

char *SRProtocol::receiveMessage(string fileName) {
	int datagram_seqn;
	char payload;

	if (c == NULL) {
		throw std::exception();
	}
	char *buf;
	buf = new char[BUF_SIZE];
	string newfile = "recvData/";
	newfile.append(fileName.substr(0, fileName.size() - 1));
	FILE *file = fopen(newfile.c_str(), "w");
	int total = 0;
	int len = c->blocking_receive(buf);
	while (len != -1) {
		istringstream ss(buf);
		int ackno;
		ss >> ackno;
		stringstream ssack;
		ssack << ackno << " ";
		string mes = buf;
		string data = mes.substr(mes.find(' ') + 1);
		cout << "\033[1;34mreceived packet no" << ackno
				<< " size=" << len << " datasize=" << data.size()
				<< "\033[0m"
				/*<< "\n"<<data<<"=============="*/<< endl;

		if (ackno - recv_base < windowsize) {
			Packet newPack = Packet();
			newPack.data = data;
			newPack.seqno = ackno;
			newPack.len = len - ssack.str().size();
			window.push_back(newPack);
			sort(window.begin(), window.end(), sortA);
			if (recv_base == ackno) {
				bool found = false;
				cout << "writing in file " << window.size()
						<< "pcks:";
				for (unsigned i = 0; i < window.size(); i++) {
					if (window[i].seqno - recv_base < 1) {
						fwrite(window[i].data.c_str(),
								sizeof(char), window[i].len,
								file);
						filesize -= window[i].len;
						cout << window[i].seqno << " size="
								<< window[i].len << " actual="
								<< window[i].data.size() << ", ";
						recv_base = window[i].seqno + 1;
					} else {
						found = true;
						window.erase(window.begin(),
								window.begin() + i);
						break;
					}
				}
				if (not found) {
					window.clear();
				}
				cout << endl;
			}

			sendAck(ackno, c->getRecvAddr());
		} else {
			cout << "packet " << ackno << " ignored" << endl;
		}
		if (filesize <= 0)
			break;
		memset(buf, 0, BUF_SIZE);
		len = c->blocking_receive(buf);
	}
//	myfile.close();
	fclose(file);
	return buf;
}
