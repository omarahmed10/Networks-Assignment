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
    // char **header = (char**)calloc(10,sizeof(char *));
    // memset(header,0,sizeof(header));
    // int requestLen = receiveMessageFromSocket(clntSocket, header);
    // for(int i = 0 ; i < requestLen; i++){
    //     printf("%s\n",header[i]);
    // }
    // char *GETLine[3];
    // split_string(header[0]," ",GETLine);
    FILE *fp = openFile("/ServerData/home.html");

    char **bodyBuff = (char**)calloc(10,sizeof(char *));
    memset(bodyBuff,0,sizeof(bodyBuff));
    
    int bodyLen = 0,bodySize = 0;
    printf("test1\n");
    bodyBuff[bodyLen] = (char*)malloc(MAXSIZE*sizeof(char));
    while(fgets(bodyBuff[bodyLen], 255, fp) != NULL){
        bodySize += strlen(bodyBuff[bodyLen]);
        printf("getting line :%s",bodyBuff[bodyLen]);
        bodyLen++;
        bodyBuff[bodyLen] = (char*)malloc(MAXSIZE*sizeof(char));
    }
    fclose(fp);
    // char *bodyData[7] = {
    //     "HTTP/1.1 200 OK\r\n",
    //     "Date: ...\r\n",
    //     "Server: Apache/2.0.45\r\n",
    //     "Last-Modified: ...\r\n",
    //     "KSDAFASODFMASDFASDF\n",
    //     "Content-Type: text/html\r\n",
    //     "\r\n"
    // };
    char CONTENT_LEN_str[80];
    sprintf(CONTENT_LEN_str, "Content-length: %d\r\n", bodySize);
    char *raw_request[7] = {
        "HTTP/1.1 200 OK\r\n",
        "Date: ...\r\n",
        "Server: Apache/2.0.45\r\n",
        "Last-Modified: ...\r\n",
        CONTENT_LEN_str,
        "Content-Type: text/html\r\n",
        "\r\n"
    };
    int headerSize = 0;
    for(int i = 0; i < 7 ;i++){
        headerSize += strlen(raw_request[i]);
    }

    printf("Sending response \n");
    sendMessageThroughSocet(clntSocket, raw_request, headerSize);
    printf("Sending response \n");
    sendMessageThroughSocet(clntSocket, bodyBuff, bodyLen);
    printf("closing \n");
    close(clntSocket);
}