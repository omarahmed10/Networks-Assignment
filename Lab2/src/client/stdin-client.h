#ifndef _STDIN_CLIENT_H
#define _STDIN_CLIENT_H 

#include "../common.h"

/* Shell client that reads from stdin and interacts with the connection c
	via Protocol p */
class StdinClient {
private:
	Protocol *p;
	Connection *c;

	char sendline[BUF_SIZE];
	
public:
	StdinClient(Protocol *p, Connection *c);
	void start();
};
#endif