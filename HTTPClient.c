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

    servPort = atoi(argv[2]); /* Use given port, if any */

    FILE *commandFile = openFile(argv[3],"r");
    while (fgets(command, 255, commandFile) != NULL){
        /* Create a reliable, stream socket using TCP */
        if ((client_sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
            DieWithError(" socket () failed");
        ConnectToTCPServer(client_sock, servlP, servPort);

        struct Command new_command = parse_command(command);
        if(new_command.is_post){
            char requestHeader[MAXSIZE];
            sprintf(requestHeader,
            "POST %s HTTP/1.1\r\nHost: %s\r\nConnection: keep-alive\r\nUser-Agent: Mozilla/5.0\r\nAccept: text/html\r\n\r\n",
            new_command.file_path,new_command.host_name);
            sendMessageThroughSocet(client_sock, requestHeader, strlen(requestHeader));

            receiveResponseFromSocket(client_sock,NULL); // receiving the OK message.

            FILE *fp = openFile(new_command.file_path,"r");
            char buffer[MAXSIZE];
            while(fgets(buffer, MAXSIZE, fp) != NULL){
                int responseLen = strlen(buffer); /* Determine input length */
                /* Send the string to the server */
                if (send(client_sock, buffer, responseLen, 0) != responseLen)
                    DieWithError("send() sent a different number of bytes than expected");
            }
            fclose(fp);

        }else{
            char requestHeader[MAXSIZE];
            sprintf(requestHeader,
            "GET %s HTTP/1.1\r\nHost: %s\r\nConnection: keep-alive\r\nUser-Agent: Mozilla/5.0\r\nAccept: text/html\r\n\r\n",
            new_command.file_path,new_command.host_name);
            sendMessageThroughSocet(client_sock, requestHeader, strlen(requestHeader));

            char download_file_final_path[MAXSIZE] = "ClientData/";
            char **download_file_path = (char **)malloc(255*sizeof(char *));
            int count = split_string(new_command.file_path,"/",download_file_path);
            strcat(download_file_final_path,download_file_path[count-1]);
            FILE *downloadFile = openFile(download_file_final_path,"w");
            receiveResponseFromSocket(client_sock,downloadFile);
            if (downloadFile != NULL) fclose(downloadFile);
        }
        printf("closing connection\n");
        close(client_sock);
    }
    
    exit(0);
}