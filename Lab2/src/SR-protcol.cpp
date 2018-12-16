/*
 * SR-protcol.cpp
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
#include <sys/time.h>
#include <algorithm>
#include <math.h>
#include "SR-protocol.h"

/* c/c++ % computes remainder, not modulo, the formula below
 returns modulo. */
#define MOD(x, m) (x % m + m) % m
#define MIN(a, b) \
    ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a < _b ? _a : _b; })

u_int32_t add(u_int16_t seqno, u_int16_t mesLen,
		const char buff[]) {
	u_int16_t padd = mesLen % 2;
	u_int16_t word16;
	u_int32_t sum = 0;

	// cast data from char to unsigned short
	u_int16_t mes[mesLen + padd];
	for (unsigned i = 0; i < mesLen; i++) {
		mes[i] = buff[i];
	}
	if (padd)
		mes[mesLen] = 0;

	// make 16 bit words out of every two adjacent 8 bit words and
	// calculate the sum of all 16 bit words
	for (unsigned i = 0; i < mesLen + padd; i = i + 2) {
		word16 = ((mes[i] << 8) & 0xFF00) + (mes[i + 1] & 0xFF);
		sum += (u_int32_t) word16;
	}
	// the seq number and the length of the data
	sum += seqno + mesLen;

	return sum;
}

u_int16_t carryComp(u_int32_t sum) {
	// keep only the last 16 bits of the 32 bit calculated sum and add the carries
	while (sum >> 16)
		sum = (sum & 0xFFFF) + (sum >> 16);
	// Take the one's complement of sum
	sum = ~sum;

	return sum;
}

u_int16_t getChecksum(u_int16_t seqno, u_int16_t mesLen,
		const char buff[]) {
	u_int32_t sum = add(seqno, mesLen, buff);

	return carryComp(sum);
}

bool isValid(u_int16_t seqno, u_int16_t mesLen,
		const char buff[], u_int16_t checksum) {
	u_int32_t sum = add(seqno, mesLen, buff);
	sum += checksum;

	return !carryComp(sum);
}

long int nowTime() {
	struct timeval tp;
	gettimeofday(&tp, NULL);
	return tp.tv_sec /*+ (1.0 / 1000000) * tp.tv_usec*/;
}
string print(vector<Packet> v) {
	string s = "";
	for (int i = 0; i < v.size(); i++) {
		s.append(NumberToString<int>(v[i].seqno));
		s.append(",");
	}
	return s;
}
string print(vector<int> v) {
	string s = "";
	for (int i = 0; i < v.size(); i++) {
		s.append(NumberToString<int>(v[i]));
		s.append(",");
	}
	return s;
}

SRProtocol::SRProtocol() {
	c = NULL;
	if (pthread_mutex_init(&base_lock, NULL) != 0) {
		printf("\n mutex init has failed\n");
		exit(-1);
	}
	if (pthread_mutex_init(&connection_lock, NULL) != 0) {
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
	sprintf(pTemp, "%d %d %s", seqno, getChecksum(seqno, len, p),
			p);
	float ran = (float) (rand() % 100) / 100;
	if (ran <= error) {
		cout << "packet no" << seqno << " is lost" << endl;
		return -1;
	} else {
		cout << "\033[1;34mseding packet no" << seqno << " size="
				<< (ss.str().size() + len) << "\033[0m" << endl;
		return c->send(pTemp,
				MIN(BUF_SIZE, ss.str().size() + len), toAddr);
	}
}

int SRProtocol::sendRequest(char *line,
		struct sockaddr_in toAddr) {
	for (;;) {
		sendDatagram(line, strlen(line), -1, toAddr);
		if (acceptAcks()) {
			break;
		}
	}
	return filesize;
}

void SRProtocol::addACK(int seq_no) {
	if (seq_no < send_base)
		return;
	std::vector<int>::iterator it;
	it = find(ACKs.begin(), ACKs.end(), seq_no);
	if (it != ACKs.end()) {
		return;
	}
	ACKs.push_back(seq_no);
	sort(ACKs.begin(), ACKs.end());
//	cout << "addACK " << seq_no << " base " << send_base
//			<< " old:" << print(ACKs) << endl;
	if (seq_no == send_base) {
		bool found = false;
		for (unsigned i = 0; i < ACKs.size(); i++) {
			if (ACKs[i] - send_base >= 1) {
				ACKs.erase(ACKs.begin(), ACKs.begin() + i);
				found = true;
				break;
			}
			num_active--;
			send_base++;
		}
		if (!found) {
			send_base = ACKs.back() + 1;
			ACKs.clear();
		}
	}
//	cout << " newbase " << send_base << " new:" << print(ACKs)
//			<< endl;
}

bool SRProtocol::checktimeout(map<int, bool> ack_mem) {
	bool found = false;
	vector<int> acked;
	for (map<int, Packet>::iterator it = allPackets.begin();
			it != allPackets.end(); it++) {
		if (ack_mem[it->first]) { // packet i is  ACKed.
			addACK(it->first);
			acked.push_back(it->first);
		} else { // packet i is timeout
			if (nowTime() - it->second.send_time >= timeout) {
				it->second.send_time = nowTime();
				sendDatagram(it->second.data.c_str(),
						it->second.len, it->first,
						it->second.addr);
			}
			found = true;
		}
	}
	for (vector<int>::iterator it = acked.begin();
			it != acked.end(); it++) {
		allPackets.erase(*it);
	}
	acked.clear();
	return found;
}

void SRProtocol::sendFile(string fileName, void* arg) {
	ThreadClientData carg = *((ThreadClientData *) arg);
	ifstream myfile(fileName, ios::in | ios::binary | ios::ate);
	if (myfile.is_open()) {
		char *memblock;
		memblock = new char[MAX_DATAGRAM_SIZE];
		int size = myfile.tellg();
		packNo = ceil(size / MAX_DATAGRAM_SIZE);
		myfile.seekg(0, ios::beg);
		cout << "server will send " << packNo << "packets"
				<< endl;
		int totalSize = 0;
		srand(carg.p->seed);
		while (myfile) {

			checktimeout((*carg.sh_mem)[carg.hash]);

			if (num_active >= windowsize) { // window is full cannot send a new packet.
				continue;
			}

			memset(memblock, 0, MAX_DATAGRAM_SIZE);
			myfile.read(memblock, MAX_DATAGRAM_SIZE);
			int i = myfile.gcount();
			totalSize += i;

			if (i) {
				num_active++;
				(*carg.sh_mem)[carg.hash][next_seq_num] = false;
				Packet p(carg.addr, next_seq_num, memblock, i,
						nowTime());
				allPackets[next_seq_num] = p;
				sendDatagram(memblock, i, next_seq_num,
						carg.addr);
//				next_seq_num = MOD((next_seq_num + 1),
//						windowsize);
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
			continue;//shouldn't happen.

		string data = mes.substr(pos, std::string::npos);

		bool valid = isValid(ackno, len - ssack.str().size(),
				mes.c_str(), checksum);
		if (!valid) {
			cout << "\033[1;31mreceived corrupted packet no"
					<< ackno << "\033[0m" << endl;
		}

		cout << "\033[1;34mreceived packet no" << ackno
				<< " size=" << len << " datasize="
				<< len - ssack.str().size() << "\033[0m"
				/*<< "\n"<<data<<"=============="*/<< endl;
		if (ackno < recv_base) {
//			cout << "\033[1;31mreceived packet no" << ackno
//					<< " size=" << len << " datasize="
//					<< data.size() << "\033[0m" << endl;
			continue;
		}
		if (ackno - recv_base < windowsize) {
			Packet newPack = Packet();
			newPack.data = data;
			newPack.seqno = ackno;
			newPack.len = len - ssack.str().size();

			std::vector<Packet>::iterator it;
			it = find(window.begin(), window.end(), newPack);
			if (it != window.end()) {
//				cout << "\033[1;33mpacket exist\033[0m" << endl;
				continue;
			}

			window.push_back(newPack);
			sort(window.begin(), window.end(), sortA);
			if (recv_base == ackno) {
				bool found = false;
				cout << "current window " << recv_base << ":"
						<< window.size() << ":" << print(window)
						<< " writing :";
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
				cout << " next " << recv_base << endl;
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
