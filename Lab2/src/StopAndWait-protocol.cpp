//#include <stdio.h>
//#include <cstdlib>
//#include <cstring>
//#include <errno.h>
//#include <exception>
//#include <sys/time.h>
//#include "StopAndWait-protocol.h"
//
//#define MIN(a, b) \
//    ({ __typeof__ (a) _a = (a); \
//       __typeof__ (b) _b = (b); \
//     _a < _b ? _a : _b; })
//
//StopAndWaitProtocol::StopAndWaitProtocol() {
//	c = NULL;
//}
//
//int StopAndWaitProtocol::sendDatagram(char *p, int len,
//		int seqno, struct sockaddr_in toAddr) {
//	char *pTemp;
//	pTemp = new char[BUF_SIZE];
//	stringstream ss;
//	ss << seqno << " ";
//	cout << "seding packet no" << seqno << " size="
//			<< (ss.str().size() + len) << endl;
//	sprintf(pTemp, "%d %s", seqno, p);
//	int sendsize = c->send(pTemp,
//			MIN(BUF_SIZE, ss.str().size() + len), toAddr);
//	return sendsize;
//}
//
//int StopAndWaitProtocol::sendRequest(char *line,
//		struct sockaddr_in toAddr) {
//	int sendsize = -1;
//	for (;;) {
//		sendsize = sendDatagram(line, strlen(line), -1, toAddr);
//		if (acceptAcks()) {
//			break;
//		}
//	}
//	return sendsize;
//}
//
//void StopAndWaitProtocol::sendFile(string fileName, string hash,
//		struct sockaddr_in toAddr) {
//	ifstream myfile(fileName, ios::in | ios::binary | ios::ate);
//	srand(7.0);
//	if (myfile.is_open()) {
//		char *memblock;
//		memblock = new char[MAX_DATAGRAM_SIZE];
//		int size = myfile.tellg();
//		myfile.seekg(0, ios::beg);
//		int totalSize = 0;
//		while (myfile) {
//			memset(memblock, 0, MAX_DATAGRAM_SIZE);
//			myfile.read(memblock, MAX_DATAGRAM_SIZE);
//			int i = myfile.gcount();
//			totalSize += i;
//			if (i) {
//				sendPacket(hash, memblock, i, toAddr);
//				//			sleep(10);
//			}
//		}
//		cout << "finish reading file " << totalSize << endl;
//		myfile.close();
//	} else {
//		cout << "file not found" << endl;
//	}
//}
//
//int StopAndWaitProtocol::sendPacket(string hash, char *line,
//		int t, struct sockaddr_in toAddr) {
//	int sendsize = -1;
//	(*sh_mem)[hash][last_seq_no] = Packet();
//	(*sh_mem)[hash][last_seq_no].data = line;
//	(*sh_mem)[hash][last_seq_no].seqno = last_seq_no;
//	for (int i = 0; i < 10; i++) {
//		struct timeval tp;
//		gettimeofday(&tp, NULL);
//		long int s = tp.tv_sec;
//		float ran = (float)(rand() % 100) / 100;
//		cout << "prob of sending=" << ran << endl;
//		if (ran <= 0.4) {
//			cout << "packet no" << last_seq_no << " is lost"
//					<< endl;
//		} else {
//			sendsize = sendDatagram(line, t, last_seq_no,
//					toAddr);
//		}
//		bool acked = false;
//		while (!acked) {
//			struct timeval tp;
//			gettimeofday(&tp, NULL);
//			long int nows = tp.tv_sec;
//			if ((*sh_mem)[hash][last_seq_no].is_ACK) { // acked
//				last_seq_no++;
//				acked = true;
//				break;
//			}
//			if (nows - s >= 5) {
//				break;
//			}
//		}
//		if (acked) {
//			break;
//		}
//	}
//	return sendsize;
//}
//
//bool StopAndWaitProtocol::acceptAcks() {
//	bool didNotTimeout;
//
//	c->setTimeout(10);
//	didNotTimeout = listenForAck();
//	c->setTimeout(0);
//
//	return didNotTimeout;
//}
//
//bool StopAndWaitProtocol::listenForAck() {
//	int mesg_seqn;
//
//	/* listen for an incoming datagram, stop if it times out */
//	cout << "waiting for ACK...." << endl;
//	char buf[BUF_SIZE];
//	if (c->blocking_receive(buf) == -1) {
//		printf("TIMEOUT\n");
//		return false;
//	}
//	istringstream iss(buf); // 0000 ACK   /// 000 GET asdfasdf
//	iss >> filesize;
//	return true;
//}
//
//void StopAndWaitProtocol::sendAck(int ackno,
//		struct sockaddr_in toAddr) {
//	char *pTemp;
//	pTemp = new char[BUF_SIZE];
//	sprintf(pTemp, "%d %s", ackno, ACK_IDENT);
//	c->send(pTemp, strlen(pTemp), toAddr);
//}
//
//char *StopAndWaitProtocol::receiveMessage(string fileName) {
//	int datagram_seqn;
//	char payload;
//
//	if (c == NULL) {
//		throw std::exception();
//	}
//	cout << "blocking for receive file of size " << filesize
//			<< endl;
//	char *buf;
//	buf = new char[BUF_SIZE];
////	ofstream myfile("recvData/" + fileName,
////			ios::out | ios::binary);
////	if (!myfile.is_open()) {
////		return NULL;
////	}
//	string newfile = "recvData/";
//	newfile.append(fileName.substr(0, fileName.size() - 1));
//	FILE *file = fopen(newfile.c_str(), "wb");
//	int total = 0;
//	int len = c->blocking_receive(buf);
//	while (len != -1) {
//		istringstream ss(buf);
//		int ackno;
//		ss >> ackno;
//		stringstream ssack;
//		ssack << ackno << " ";
//		string mes = buf;
//		string data = mes.substr(mes.find(' ') + 1);
////		myfile << data;
//
//		fwrite(data.c_str(), sizeof(char),
//				len - ssack.str().size(), file);
//
//		cout << "received packet no" << ackno << " size=" << len
//				<< " datasize=" << data.size() << endl;
//		sendAck(ackno, c->getRecvAddr());
//
//		filesize -= (len - ssack.str().size());
//		total += (len - ssack.str().size());
//		if (filesize <= 0)
//			break;
//		memset(buf, 0, BUF_SIZE);
//		len = c->blocking_receive(buf);
//	}
//	cout << "recv size " << total << endl;
////	myfile.close();
//	fclose(file);
//	return buf;
//}
