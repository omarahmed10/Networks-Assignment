/*
 * gobackn-protcol.cpp
 *
 *  Created on: Dec 8, 2018
 *      Author: omar
 */
#include <stdio.h>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <errno.h>
#include <exception>
#include <algorithm>
#include <math.h>
#include "gobackn-protocol.h"

/* c/c++ % computes remainder, not modulo, the formula below
 returns modulo. */
#define MOD(x, m) (x % m + m) % m
#define MIN(a, b) \
    ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a < _b ? _a : _b; })

void GoBackNProtocol::readCongList() {
	ifstream infile("congestion.in");
	while (infile) {
		int n;
		infile >> n;
		threeACKpck.push(n);
	}
}

GoBackNProtocol::GoBackNProtocol() {
	c = NULL;
}

int GoBackNProtocol::sendDatagram(const char *p, int len, int seqno,
		struct sockaddr_in toAddr) {
	int checksum = getChecksum(seqno, len, p);
	char *pTemp;
	pTemp = new char[BUF_SIZE];
	stringstream ss;
	ss << seqno << " " << checksum << " ";
	sprintf(pTemp, "%d %d %s", seqno, checksum, p);
	float ran = (float) (rand() % 100) / 100;
	if (ran <= error) {
		cout << "packet no" << seqno << " is lost" << endl;
		return -1;
	} else {
		cout << "\033[1;34mseding packet no" << seqno << " size="
				<< (ss.str().size() + len) << " windowsize " << windowsize
				<< "\033[0m" << endl;
		return c->send(pTemp, MIN(BUF_SIZE, ss.str().size() + len), toAddr);
	}
}

int GoBackNProtocol::sendRequest(char *line, struct sockaddr_in toAddr) {
	for (;;) {
		sendDatagram(line, strlen(line), -1, toAddr);
		if (acceptAcks()) {
			break;
		}
	}
	return filesize;
}

void GoBackNProtocol::addACK(int seq_no) {
	if (seq_no < send_base)
		return;
	send_base = seq_no + 1;
	if (send_base != next_seq_num
			&& allPackets.find(send_base) != allPackets.end()) {
		allPackets[send_base].send_time = nowTime();
	}
	if (!threeACKpck.empty() && send_base == threeACKpck.front()) { // fast recovery
		cout << "\033[1;35mfast recovery\033[0m" << endl;
		threeACKpck.pop();
		ssthresh = windowsize / 2;
		windowsize = ssthresh + 3;
	} else if (windowsize >= ssthresh) { // congestion avoidance
		cout << "\033[1;33mcongestion avoidance\033[0m" << endl;
		windowsize += (1.0 / windowsize);
	} else { // slow start.
		cout << "\033[1;33mslow start\033[0m" << endl;
		windowsize++;
		if (windowsize > ssthresh)
			windowsize = ssthresh;
	}
}

bool GoBackNProtocol::checktimeout(map<int, bool> ack_mem) {
	bool found = false;
	vector<int> acked;
	for (map<int, Packet>::iterator it = allPackets.begin();
			it != allPackets.end(); it++) {
		if (ack_mem[it->first]) { // packet i is  ACKed.
			addACK(it->first);
			acked.push_back(it->first);
		} else if (it->first == send_base) { // as long as the base packet is exist in
											 //	allPackets map we have to continue checking.
			if (nowTime() - it->second.send_time >= timeout) { // the first not ACK packet is timed out
				losscnt++;
				ssthresh = windowsize / 2;
				windowsize = 1;
				cout << "\033[1;31mTIME OUT for " << it->first << " ssthresh "
						<< ssthresh << "\033[0m" << endl;
				it->second.send_time = nowTime();
				for (map<int, Packet>::iterator it2 = allPackets.begin();
						it2 != allPackets.end(); it2++) {
					if (it2->first >= it->first) {
						sendDatagram(it2->second.data.c_str(), it2->second.len,
								it2->first, it2->second.addr);
					}
				}
				found = true;
				break; // don't no other packet is important because they will be resend again.
			}
			found = true;
		}
	}
	for (vector<int>::iterator it = acked.begin(); it != acked.end(); it++) {
		allPackets.erase(*it);
	}
	acked.clear();
	return found;
}

void GoBackNProtocol::sendFile(string fileName, void* arg) {
	ThreadClientData carg = *((ThreadClientData *) arg);
	readCongList();
	ifstream myfile(fileName, ios::in | ios::binary | ios::ate);
	if (myfile.is_open()) {
		char *memblock;
		memblock = new char[MAX_DATAGRAM_SIZE];
		int size = myfile.tellg();
		packNo = ceil(size / MAX_DATAGRAM_SIZE);
		myfile.seekg(0, ios::beg);
		cout << "server will send " << packNo << "packets" << endl;
		int totalSize = 0;
		ssthresh = windowsize;
		windowsize = 1.0;
		srand(carg.p->seed);
		while (myfile) {

			checktimeout((*carg.sh_mem)[carg.hash]);

			if (next_seq_num - send_base >= trunc(windowsize)) { // window is full cannot send a new packet.
				continue;
			}

			memset(memblock, 0, MAX_DATAGRAM_SIZE);
			myfile.read(memblock, MAX_DATAGRAM_SIZE);
			int i = myfile.gcount();
			totalSize += i;

			if (i) {
				(*carg.sh_mem)[carg.hash][next_seq_num] = false;
				Packet p(carg.addr, next_seq_num, memblock, i, nowTime());
				allPackets[next_seq_num] = p;
				sendDatagram(memblock, i, next_seq_num, carg.addr);
				next_seq_num++;
			}
		}
		while (checktimeout((*carg.sh_mem)[carg.hash])) {
		}
		allPackets.clear();
		cout << "finish reading file " << totalSize << endl;
		myfile.close();
	} else {
		cout << "file not found" << endl;
	}
}

bool GoBackNProtocol::acceptAcks() {
	bool didNotTimeout;

	c->setTimeout(5);
	didNotTimeout = listenForAck();
	c->setTimeout(0);

	return didNotTimeout;
}

bool GoBackNProtocol::listenForAck() {
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

void GoBackNProtocol::sendAck(int ackno, struct sockaddr_in toAddr) {
	char *pTemp;
	pTemp = new char[BUF_SIZE];
	sprintf(pTemp, "%d %s", ackno, ACK_IDENT);
	c->send(pTemp, strlen(pTemp), toAddr);
}

char *GoBackNProtocol::receiveMessage(string fileName) {
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
		int ackno, checksum;
		ss >> ackno >> checksum;

		stringstream ssack;
		ssack << ackno << " " << checksum << " ";
		string mes = buf;
		size_t pos = mes.find(" ");
		if (pos == std::string::npos)
			continue; //shouldn't happen.
		pos = mes.find(" ", pos + 1);
		if (pos == std::string::npos)
			continue; //shouldn't happen.

		string data = mes.substr(pos + 1, std::string::npos);

		bool valid = isValid(ackno, len - ssack.str().size(), data.c_str(),
				checksum);
		if (!valid) {
			cout << "\033[1;31mreceived corrupted packet no" << ackno
					<< "\033[0m" << endl;
		}

		cout << "\033[1;34mreceived packet no" << ackno << " size=" << len
				<< " datasize=" << len - ssack.str().size() << "\033[0m"
				<< endl;
		if (ackno == recv_base) {
			fwrite(data.c_str(), sizeof(char), len - ssack.str().size(), file);
			filesize -= len - ssack.str().size();
			recv_base++;

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

/*
 *   const std::string str = "Jhon 12345 R333445 3434";

 size_t pos = str.find(" ");
 if (pos == std::string::npos)
 return -1;

 pos = str.find(" ", pos + 1);
 if (pos == std::string::npos)
 return -1;

 std::cout << str.substr(pos, std::string::npos);
 */
