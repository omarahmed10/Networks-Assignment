#ifndef _STDOUT_SERVER_H
#define _STDOUT_SERVER_H

#include "../common.h"

/* shell program that receives input from Connection c using Protocol p
	and outputs to stdout */
class StdoutServer {
private:
	Protocol *p;
	Connection *c;

public:
	StdoutServer(Protocol *p, Connection *c);
	void start();
};

#endif