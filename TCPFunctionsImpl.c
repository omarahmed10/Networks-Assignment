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

void sendMessageThroughSocet(int sock, char* message, int len){
    if (send(sock, message, len, 0) != len)
        DieWithError("send() sent a different number of bytes than expected");
    printf("Done Sending\n");
}

void receiveResponseBodyFromSocket(int sock, FILE *outputFile){
    int recvMsgSize;

    char buffer[MAXSIZE];
    memset(&buffer,0,MAXSIZE);
    if ((recvMsgSize = recv(sock, buffer, MAXSIZE-1, 0)) < 0)
        DieWithError("recv() failed") ;
    buffer[recvMsgSize] = '\0';

    while (recvMsgSize > 0) {
        fprintf(outputFile,"%s",buffer);
        /* See if there is more data to receive */
        memset(&buffer,0,MAXSIZE);
        if ((recvMsgSize = recv(sock, buffer, MAXSIZE-1, 0)) < 0)
            DieWithError("recv() failed") ;
        buffer[recvMsgSize] = '\0';
    }
}

void receiveResponseFromSocket(int sock, FILE *outputFile){
    int recvMsgSize;
    char buffer[MAXSIZE];

    memset(&buffer,0,MAXSIZE);
    if ((recvMsgSize = recv(sock, buffer, MAXSIZE-1, 0)) < 0)
        DieWithError("recv() failed") ;
    buffer[recvMsgSize] = '\0';

    int status = 0,len = 0;
    bool bodyStart = false;
    while (recvMsgSize > 0) {
        if(!bodyStart){
            char buff_cpy[MAXSIZE];
            strcpy(buff_cpy,buffer);
            status = atoi(str_find_next(buff_cpy,"HTTP/1.1",""));
            if(status == 404){
                printf("%s",buffer);
                break;
            }
            strcpy(buff_cpy,buffer);
            char *len_ch = str_find_next(buff_cpy, "Content-length:","");
            if(len_ch != NULL){
                len = atoi(len_ch);
            }

            strcpy(buff_cpy,buffer);
            if(!len){/* incase no body or status 404 found 
                then we have to terminate when empty line found in header. */
                printf("%s",buffer);
                break;
            }else{
                char **strings = (char **)malloc(MAXSIZE*sizeof(char *));
                int string_count = split_string(buffer, "\n",strings);
                for(int i = 0 ;i < string_count;i++){
                    if(!bodyStart && strlen(strings[i]) == 1 && strings[i][0] == 13){
                        bodyStart = true;
                    }else if (bodyStart){
                        fprintf(outputFile,"%s",strings[i]);
                    }else{
                        printf("%s\n",strings[i]);
                    }
                }
            }
        }else{
            fprintf(outputFile,"%s",buffer);
        }

        /* See if there is more data to receive */
        memset(&buffer,0,MAXSIZE);
        if ((recvMsgSize = recv(sock, buffer, MAXSIZE-1, 0)) < 0)
            DieWithError("recv() failed") ;
        buffer[recvMsgSize] = '\0';
    }
}
short operation = 0;
char *receiveRequestHeaderFromSocket(int sock){
    operation = 0;
    int recvMsgSize;
    char buffer[MAXSIZE];
    memset(&buffer,0,MAXSIZE);
    if ((recvMsgSize = recv(sock, buffer, MAXSIZE-1, 0)) < 0)
        DieWithError("recv() failed") ;
    buffer[recvMsgSize] = '\0';

    char *filepath = NULL;
    while (recvMsgSize > 0) {
        printf("%s",buffer);
        char buff_cpy[MAXSIZE];
        strcpy(buff_cpy,buffer);
        if(filepath == NULL){
            filepath = str_find_next(buff_cpy,"GET","POST");
            operation = str_exist(buff_cpy,"GET","POST");
        }
        if (str_find_empty_line(buffer)){
            break;
        }
        /* See if there is more data to receive */
        memset(&buffer,0,MAXSIZE);
        if ((recvMsgSize = recv(sock, buffer, MAXSIZE-1, 0)) < 0)
            DieWithError("recv() failed") ;
        buffer[recvMsgSize] = '\0';
    }
    if (filepath != NULL){
        static char ans[MAXSIZE];
        strcpy(ans,filepath);
        return ans;
    }
    return NULL;
}