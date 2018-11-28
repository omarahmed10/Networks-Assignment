// #include <stdio.h>
// #include <sys/socket.h>
// #include <arpa/inet.h>
// #include <stdlib.h>
// #include <string.h>
// #include <unistd.h>
// #include <fcntl.h>
// #include <sys/file.h>
// #include <signal.h>
// #include <errno.h>

// #define ECHOMAX 255
// #define MAXPENDING 5
// #define RCVBUFSIZE 32

// void DieWithError(char *errorMessage);
// void HandleTCPClient(int clntSocket);
// void UseldleTime();
// void SlGlOHandler(int signalType);

// int sock; /* Socket -- GLOBAL for signal handler */

// int main(int argc, char *argv[])
// {
//     // int servSock;
//     int clntSock;
//     struct sockaddr_in echoServAddr;
//     struct sockaddr_in echoClntAddr;
//     unsigned short echoServPort;
//     unsigned int clntLen;
//     struct sigaction handler; /* Signal handling action definition */

//     if (argc != 2)
//     {
//         fprintf(stderr, "Usage: %s <Server Port>\n", argv[0]) ;
//         exit(1);
//     }

//     echoServPort = atoi(argv[1]);
//     printf("creating socket\n");
//     if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
//         DieWithError( "socket () failed") ;
//     printf("Done socket\n");
//     /* Construct local address structure */
//     memset(&echoServAddr, 0, sizeof(echoServAddr));
//     echoServAddr.sin_family = AF_INET;
//     echoServAddr.sin_addr.s_addr = htonl(INADDR_ANY);
//     echoServAddr.sin_port = htons(echoServPort);
//     printf("server IP %s\n",inet_ntoa(echoServAddr.sin_addr));

//     if (bind(sock, (struct sockaddr *)&echoServAddr,sizeof(echoServAddr)) < 0)
//         DieWithError ( "bind () failed");

//     handler.sa_handler = SIGlOHandler;
//     /* Create mask that masks all signals */
//     if (sigfillset(&handler.sa_mask) < 0)
//         DieWithError("sigfillset() failed");
//     /* No flags */
//     handler.sa_flags = 0;
//     /* We must own the socket to receive the SIGIO message */
//     if (fcntl(sock, F_SETOWN, getpid()) < 0)
//         DieWithError("Unable to set process owner to us");
//     /* Arrange for nonblocking I/0 and SIGIO delivery */
//     if (fcntl(sock, F_SETYL, O_NONBLOCK | FASYNC) < 0)
//         DieWithError("Unable to put client sock into nonblocking/async mode");
//     /* Go off and do real work; echoing happens in the background */
//     for (; ;)
//         UseldleTime() ;
// }

// //     /* Mark the socket so it will listen for incoming connections */
// //     if (listen(servSock, MAXPENDING) < 0)
// //         DieWithError("listen() failed") ;
// //     printf("Done biniding\n");
// //     for (;;) /* Run forever */
// //     {
// //         /* Set the size of the in-out parameter */
// //         clntLen = sizeof(echoClntAddr);
// //         /* Wait for a client to connect */
// //         if ((clntSock = accept(servSock, (struct sockaddr *) &echoClntAddr, &clntLen)) < 0)
// //             DieWithError("accept() failed");
// //         /* clntSock is connected to a client! */
// //         printf("Handling client %s\n", inet_ntoa(echoClntAddr.sin_addr));

// //         HandleTCPClient (clntSock);
// //     }
// //     HandleTCPClient (clntSock) ;
// //     /* NOT REACHED */
// // }
// void UseldleTime()
//  {
//     printf(".\n");
//     sleep(3);
//     /* 3 seconds of activity */
//  }
// void SIGlOHandler(int signalType)
//  {

//     struct sockaddr_in echoClntAddr; /* Address of datagram source */
//     unsigned int clntLen; /* Address length */
//     int recvMsgSize; /* Size of datagram */
//     char echoBuffer[ECHOMAX];
//     do /* As long as there is input... */
//     {
//         clntLen = sizeof(echoClntAddr);
//         if ((recvMsgSize = recvfrom(sock, echoBuffer, ECHOMAX, 0,(struct sockaddr *) &echoClntAddr, &clntLen)) < 0)
//         {
//             if (errno != EWOULDBLOCK)
//                 DieWithError("recvfrom() failed");
//         }
//         else
//         {
//             printf("Handling client %s\n", inet_ntoa(echoClntAddr.sin_addr));
//             if (sendto(sock, echoBuffer, recvMsgSize, 0, (struct sockaddr *) &echoClntAddr, sizeof(echoClntAddr)) != recvMsgSize)
//                 DieWithError("sendto() failed");
//         }
//     } while (recvMsgSize >= 0);
//  }


// void HandleTCPClient(int clntSocket)
// {
//     char echoBuffer[RCVBUFSIZE];
//     int recvMsgSize;

//     if ((recvMsgSize = recv(clntSocket, echoBuffer, RCVBUFSIZE, 0)) < 0)
//         DieWithError("recv() failed") ;
    
//     while (recvMsgSize > 0)
//     {
//         /* Echo message back to client */
//         if (send(clntSocket, echoBuffer, recvMsgSize, 0) != recvMsgSize)
//             DieWithError("send() failed");
//         /* See if there is more data to receive */
//         if ((recvMsgSize = recv(clntSocket, echoBuffer, RCVBUFSIZE, 0)) < 0)
//             DieWithError("recv() failed") ;
//     }
//     close(clntSocket);
// }

//     /* Print a final linefeed */
// void DieWithError(char *errorMessage)
// {
//     perror ( errorMessage) ;
//     exit(1);
// }