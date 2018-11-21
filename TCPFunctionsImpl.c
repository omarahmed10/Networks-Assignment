#include "TCPFunctions.h"

int CreateTCPServerSocket(unsigned short port)
{
    int sock; /* Socket to create */
    struct sockaddr_in echoServAddr; /* Local address */

    /* Create socket for incoming connections */
    if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
        DieWithError(" socket () failed") ;
    
    /* Construct local address structure */
    memset(&echoServAddr, 0, sizeof(echoServAddr)); /*Zero out structure */
    echoServAddr.sin_family = AF_INET; /* Internet address family */
    echoServAddr.sin_addr.s_addr = htonl(INADDR_ANY); /* Any incoming interface */
    echoServAddr.sin_port = htons(port);/*Local port */

    /* Bind to the local address */
    if (bind(sock, (struct sockaddr *)&echoServAddr,sizeof(echoServAddr)) < 0)
        DieWithError ("bind () failed");
    
    /* Mark the socket so it will listen for incoming connections */
    if (listen(sock, MAXPENDING) < 0)
        DieWithError("listen() failed") ;
    
    return sock;
}

/* Print a final linefeed */
void DieWithError(char *errorMessage)
{
    perror ( errorMessage) ;
    exit(1);
}

int AcceptTCPConnection(int servSock)
{
    int clntSock; /* Socket descriptor for client */
    struct sockaddr_in echoClntAddr; /* Client address */
    unsigned int clntLen; /* Length of client address data structure */

    /* Set the size of the in-out parameter */
    clntLen = sizeof(echoClntAddr);
    /* Wait for a client to connect */
    if ((clntSock = accept(servSock, (struct sockaddr *) &echoClntAddr, &clntLen)) < 0)
        DieWithError( "accept () failed") ;
    
    /* clntSock is connected to a client! */
    printf("Handling client %s\n", inet_ntoa(echoClntAddr.sin_addr)) ;
    return clntSock;
}
// char *strcat


struct sockaddr_in ConnectToTCPServer(int clientSock, char *servlP, unsigned short servPort){
    struct sockaddr_in servAddr;

    /* Construct the server address structure */
    memset(&servAddr, 0, sizeof(servAddr)); /* Zero out structure */
    servAddr.sin_family = AF_INET; /* Internet address family */
    servAddr.sin_addr.s_addr = inet_addr(servlP); /* Server IP address */
    servAddr.sin_port = htons(servPort); /* Server port */

    /* Establish the connection to the echo server */
    if (connect(clientSock, (struct sockaddr *)&servAddr, sizeof(servAddr)) < 0)
        DieWithError(" connect () failed");
    
    return servAddr;
}

void sendMessageThroughSocet(int sock, char** message, int len){
    for(int i = 0; i < len; i++){
        int requestLen = strlen(message[i]); /* Determine input length */
        /* Send the string to the server */
        if (send(sock, message[i], requestLen, 0) != requestLen)
            DieWithError("send() sent a different number of bytes than expected");
    }
    printf("Done Sending\n");
}

int receiveMessageFromSocket(int sock, char** buff){
    int recvMsgSize, recvMsgLine = 0;
    buff[recvMsgLine] = (char *)malloc(MAXSIZE*sizeof(char));
    if ((recvMsgSize = recv(sock, buff[recvMsgLine], MAXSIZE, 0)) <= 0)
        DieWithError("recv() failed") ;
    printf("receving :%s",buff[recvMsgLine]);
    while (recvMsgSize > 0) {
        if (buff[recvMsgLine][recvMsgSize-1] == '\n' && buff[recvMsgLine][recvMsgSize-2] == '\r'){
            recvMsgLine++;
            break;
        }
        recvMsgLine++;
        /* See if there is more data to receive */
        buff[recvMsgLine] = (char *)malloc(MAXSIZE*sizeof(char));
        if ((recvMsgSize = recv(sock, buff[recvMsgLine], MAXSIZE, 0)) <= 0)
            DieWithError("recv() failed") ;
    }
    char *receivedMsg = (char*)malloc(sizeof(char *));
    for(int i = 0 ; i < recvMsgLine; i++){
        receivedMsg = strcat(receivedMsg,buff[i]);
    }
    free(buff);
    int string_count = split_string(receivedMsg,"\n",buff);
    return string_count;
}