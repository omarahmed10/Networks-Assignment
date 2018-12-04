#include <stdio.h>
#include <cstdlib>
#include <cstring>
#include <errno.h>
#include <exception>
#include "StopAndWait-protocol.h"

#define MIN(a, b) \
    ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a < _b ? _a : _b; })

StopAndWaitProtocol::StopAndWaitProtocol()
{
    c = NULL;
}

int StopAndWaitProtocol::sendDatagram(char *p)
{
    printf("SENDING: %s\n", p);
    std::cout << "size " << MIN(MAX_DATAGRAM_SIZE, strlen(p)) << std::endl;
    return c->send(p, MIN(MAX_DATAGRAM_SIZE, strlen(p)));
}

int StopAndWaitProtocol::sendMessage(char *line, unsigned int t)
{
    if (c == NULL)
    {
        throw std::exception();
    }

    int current_byte = 0;
    /* loop while we have more to send or are still waiting
		on outstanding ACKs */
    while (t > 0)
    {
        /* send as many as we can*/
        line += current_byte;
        int sendlen = sendDatagram(line);
        current_byte = 0;
        /* listen */
        if (acceptAcks())
        {
            current_byte = sendlen; // advance the line pointer when ack received.
            t -= sendlen;
        }else{
            std::cout << "NACK" << std::endl;
        }
    }

    printf("ALL BYTES SENT for line %s\n",line);
}

bool StopAndWaitProtocol::acceptAcks()
{
    bool didNotTimeout;

    c->setTimeout(5);
    didNotTimeout = listenForAck();
    c->setTimeout(0);

    return didNotTimeout;
}

bool StopAndWaitProtocol::listenForAck()
{
    int mesg_seqn;

    /* listen for an incoming datagram, stop if it times out */
    if (c->blocking_receive(buffer) == -1)
    {
        printf("TIMEOUT\n");
        return false;
    }
    return true;
}

void StopAndWaitProtocol::sendAck(char *message)
{
    sscanf(buffer, ACK_IDENT " for %s", message);
    c->send(ACK_IDENT, strlen(buffer));
}

char *StopAndWaitProtocol::receiveMessage()
{
    int datagram_seqn;
    char payload;

    if (c == NULL)
    {
        throw std::exception();
    }
    std::cout << "blocking for receive" << std::endl;
    while (c->blocking_receive(buffer) != -1)
    {
        std::cout << "sending ACK " << std::endl;
        sendAck(buffer);
    }

    return buffer;
}
