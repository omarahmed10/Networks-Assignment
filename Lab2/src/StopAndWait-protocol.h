#ifndef _SIOPANDWAIT_PROTOCOL_H
#define _SIOPANDWAIT_PROTOCOL_H
#include "common.h"

/* Implementation of the Stop-And-Wait Protocol, The server sends a single datagram, and blocks until an acknowledgment from the client is
received (or until a timeout expires). */
class StopAndWaitProtocol : public Protocol
{
  private:
    char buffer[BUF_SIZE]; //buffer used for incoming and outgoing messages

    /* use the underlying connection to send a message */
    int sendDatagram(char *p);

    /* listen for an ACK, returns true if the socket does not timeout */
    bool acceptAcks();

    /* helper function to set timeout to 5 seconds before listening for an ACK */
    bool listenForAck();

    /* send an ACK containing sequence number seqn */
    void sendAck(char *message);

  public:
    StopAndWaitProtocol();

    /* implementation of abstract Protocol::sendMessage */
    virtual int sendMessage(char *line, unsigned int t, bool recvACK);
    /* implementation of abstract Protocol::receiveMessage */
    virtual char *receiveMessage(bool sendACK);
};

#endif