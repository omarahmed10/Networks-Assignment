#include "TCPFunctions.h"

/* Size of receive buffer */
void DieWithError(char *errorMessage);
/* Error handling function */
int main(int argc, char *argv[])
{
    int client_sock;
    unsigned short servPort;
    char *servlP = argv[1];
    char command[255];

    if (argc == 3)
        servPort = atoi(argv[2]); /* Use given port, if any */
    else
        servPort = 80; /* 80 is the well-known port for the HTTP service */

    FILE *commandFile = openFile("/inputs.txt","r");
    while (fgets(command, 255, commandFile) != NULL){
        /* Create a reliable, stream socket using TCP */
        if ((client_sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
            DieWithError(" socket () failed");
        ConnectToTCPServer(client_sock, servlP, servPort);

        struct Command new_command = parse_command(command);
        if(new_command.is_post){
            printf("POST command\n");
        }else{
            char requestHeader[MAXSIZE];
            sprintf(requestHeader,
            "GET %s HTTP/1.1\r\nHost: %s\r\nConnection: keep-alive\r\nUser-Agent: Mozilla/5.0\r\nAccept: text/html\r\n\r\n",
            new_command.file_path,new_command.host_name);
            sendMessageThroughSocet(client_sock, requestHeader, strlen(requestHeader));
            FILE *outputFP = openFile("/ClientData/home.html","w");
            receiveResponseFromSocket(client_sock,outputFP);
            fclose(outputFP);
        }
        printf("closing connection\n");
        close(client_sock);
    }
    
    exit(0);
}