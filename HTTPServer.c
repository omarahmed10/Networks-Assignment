#include "TCPFunctions.h"
#include <pthread.h>

void *ThreadMain(void *arg);
void HandleHTTPClient(int clntSocket);

struct ThreadArgs
{
    int clntSock;
};

int main(int argc, char *argv[])
{
    int servSock;
    int clntSock;
    unsigned short echoServPort;
    pthread_t threadID;
    struct ThreadArgs *threadArgs;
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <SERVER PORT>\n", argv[0]) ;
        exit(1);
    }
    echoServPort = atoi(argv[1]); /* First arg' local port */
    servSock = CreateTCPServerSocket (echoServPort) ;
    for (;;) /* Run forever */
    {
        clntSock = AcceptTCPConnection(servSock) ;
        if ((threadArgs = (struct ThreadArgs *) malloc(sizeof(struct ThreadArgs))) == NULL)
            DieWithError("malloc() failed");
        threadArgs -> clntSock = clntSock;
        if (pthread_create(&threadID, NULL, ThreadMain, (void *) threadArgs) != 0)
            DieWithError("pthread_create() failed");
        printf("with thread %ld\n", (long int) threadID);
    }
}
void *ThreadMain(void *threadArgs)
{
    int clntSock;
    pthread_detach(pthread_self() ) ;
    clntSock = ((struct ThreadArgs *) threadArgs) -> clntSock;
    free(threadArgs);
    HandleHTTPClient (clntSock) ;
    return (NULL) ;
}

void HandleHTTPClient(int clntSocket)
{
    struct Req_Head requestHead = receiveRequestHeaderFromSocket(clntSocket);
    if(requestHead.is_GET == 1){
        FILE *fp = openFile(requestHead.file_path,"r");
        if (fp == NULL){
            close(clntSocket);
            return;
        }
        fseek(fp, 0L, SEEK_END);
        int file_size = ftell(fp);
        rewind(fp);
        
        char responseHeader[MAXSIZE];
        sprintf(responseHeader, 
        "HTTP/1.1 200 OK\r\nDate: ...\r\nServer: Apache/2.0.45\r\nContent-length: %d\r\nContent-Type: text/html\r\n\r\n"
        , file_size);
        printf("Sending response header \n");
        sendMessageThroughSocet(clntSocket, responseHeader, strlen(responseHeader));

        printf("Sending response body \n");
        char buffer[MAXSIZE];
        while(fgets(buffer, MAXSIZE, fp) != NULL){
            int responseLen = strlen(buffer); /* Determine input length */
            /* Send the string to the server */
            if (send(clntSocket, buffer, responseLen, 0) != responseLen)
                DieWithError("send() sent a different number of bytes than expected");
        }
        fclose(fp);
    }else{
        
    }
    

    printf("closing \n");
    close(clntSocket);
}