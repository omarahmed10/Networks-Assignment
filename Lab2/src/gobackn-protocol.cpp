//#include <stdio.h>
//#include <cstdlib>
//#include <cstring>
//#include <errno.h>
//#include <exception>
//#include "gobackn-protocol.h"
//
//GoBackNProtocol::GoBackNProtocol()
//{
//	c = NULL;
//	current_seqn = 0;
//	expected_seqn = 0;
//	last_sent_ackn = 0;
//	num_active = 0;
//	last_acked_seqn = LAST_SENT_SEQN;
//}
//
//void GoBackNProtocol::sendDatagram(char *p)
//{
//	printf("SENDING: %s\n", p);
//	c->send(p, strlen(p));
//}
//
//void GoBackNProtocol::addToWindow(char b)
//{
//	char *pTemp;
//	pTemp = new char[MAX_DATAGRAM_SIZE];
//
//	sprintf(pTemp, DATAGRAM_IDENT " %i %c", current_seqn, b);
//	window[current_seqn] = pTemp;
//	current_seqn = MOD((current_seqn + 1), N);
//	num_active++;
//
//	sendDatagram(pTemp);
//}
//
//bool GoBackNProtocol::canAddToWindow()
//{
//	//printf("current_seqn %i last_accked_seqn %i, num_active(%i)", current_seqn, last_acked_seqn, num_active);
//	return num_active < N;
//}
//
//bool GoBackNProtocol::windowEmpty()
//{
//	//printf("last_acked_seqn %i  == LAST_SENT_SEQN %i\n", last_acked_seqn, LAST_SENT_SEQN);
//	return last_acked_seqn == LAST_SENT_SEQN;
//}
//
//void GoBackNProtocol::resendWindow()
//{
//	int num_sending;
//
//	num_sending = num_active;
//	printf("\tresending %i datagrams\n", num_active);
//
//	for (int i = 1; i <= num_sending; i++)
//	{
//		int current = MOD(last_acked_seqn + i, N);
//		printf("\t");
//		sendDatagram(window[current]);
//	}
//}
//
//int GoBackNProtocol::sendMessage(char *line, unsigned int t)
//{
//	if (c == NULL)
//	{
//		throw std::exception();
//	}
//
//	int current_byte = 0;
//	/* loop while we have more to send or are still waiting
//		on outstanding ACKs */
//	while (!windowEmpty() || current_byte < t)
//	{
//
//		/* send as many as we can*/
//		//while (last_sent_seqn )
//		while (canAddToWindow() && current_byte < t)
//		{
//			addToWindow(line[current_byte]);
//			current_byte++;
//		}
//
//		/* listen */
//		if (!acceptAcks())
//		{
//			/* our listen timed out, resend the entire window */
//			resendWindow();
//		}
//	}
//
//	printf("WINDOW EMPTY & ALL BYTES SENT\n");
//}
//
//bool GoBackNProtocol::acceptAcks()
//{
//	bool didNotTimeout;
//
//	c->setTimeout(5);
//	didNotTimeout = listenForAck();
//	c->setTimeout(0);
//
//	return didNotTimeout;
//}
//
//void GoBackNProtocol::removeFromWindow(int mesg_seqn)
//{
//	//delete
//	int num_deleting;
//
//	num_deleting = MOD(mesg_seqn - last_acked_seqn, N);
//	//printf("num_deleting %i\n", num_deleting);
//
//	if (num_deleting > 0)
//	{
//		printf("\nRECEIVED: ACK %i\n", mesg_seqn);
//	}
//
//	for (int i = 1; i <= num_deleting; i++)
//	{
//		int deleting = MOD(last_acked_seqn + i, N);
//		delete window[deleting];
//		num_active--;
//		printf("\tdeleting %i from window\n", deleting);
//	}
//
//	last_acked_seqn = mesg_seqn;
//}
//
//bool GoBackNProtocol::parseValidAck(int *ack_seqn, char *data_in)
//{
//	char whitespace_char;
//
//	//printf("Parsing: '%s'\n", data_in);
//	if (!sscanf(data_in, ACK_IDENT "%c%d", &whitespace_char, ack_seqn))
//	{
//		return false;
//	}
//
//	if (!whitespace_char == ' ' || *ack_seqn < 0 || *ack_seqn >= N)
//	{
//		return false;
//	}
//
//	return true;
//}
//
//bool GoBackNProtocol::listenForAck()
//{
//	int mesg_seqn;
//
//	/* listen for an incoming datagram, stop if it times out */
//	if (c->blocking_receive(buffer) == -1)
//	{
//		printf("TIMEOUT\n");
//		return false;
//	}
//
//	/* we got something, validate it */
//	if (!parseValidAck(&mesg_seqn, buffer))
//	{
//		printf("\tBAD ACK '%s', RELISTENING\n", buffer);
//
//		/* this ACK is invalid, reset our timeout and try again */
//		return listenForAck();
//	}
//
//	/* server ACKed mesq_seqn, update our window */
//	removeFromWindow(mesg_seqn);
//
//	return true;
//}
//
//void GoBackNProtocol::sendAck(int seqn)
//{
//	last_sent_ackn = seqn;
//
//	sprintf(buffer, ACK_IDENT " %i", seqn);
//	c->send(buffer, strlen(buffer));
//}
//
//bool GoBackNProtocol::parseValidDatagram(int *datagram_seqn, char *payload, char *data_in)
//{
//	char whitespace_char, in_byte, whitespace_char2;
//
//	printf("Parsing: '%s'\n", data_in);
//	if (!sscanf(data_in, DATAGRAM_IDENT "%c%d%c%c", &whitespace_char, datagram_seqn, &whitespace_char2, payload))
//	{
//		return false;
//	}
//
//	if (!whitespace_char == ' ' || !whitespace_char2 == ' ' || *datagram_seqn < 0 || *datagram_seqn >= N)
//	{
//		return false;
//	}
//
//	if (*datagram_seqn != expected_seqn)
//	{
//		return false;
//	}
//
//	return true;
//}
//
//char *GoBackNProtocol::receiveMessage()
//{
//	int datagram_seqn;
//	char payload;
//
//	if (c == NULL)
//	{
//		throw std::exception();
//	}
//
//	while (c->blocking_receive(buffer) != -1)
//	{
//		if (parseValidDatagram(&datagram_seqn, &payload, buffer))
//		{
//			expected_seqn = MOD(expected_seqn + 1, N);
//			printf("got payload of %c\n", payload);
//			sendAck(datagram_seqn);
//		}
//		else
//		{
//			sendAck(last_sent_ackn);
//		}
//	}
//
//	return buffer;
//}
