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

    /* Create a reliable, stream socket using TCP */
    if ((client_sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
        DieWithError(" socket () failed");
    FILE *commandFile = openFile("/inputs.txt");
    while (fgets(command, 255, commandFile) != NULL){
        ConnectToTCPServer(client_sock, servlP, servPort);

        struct Command new_command = parse_command(command);
        if(new_command.is_post){
            printf("POST command\n");
        }else{
            char GET_str[80],HOST_str[80];
            sprintf(GET_str, "GET %s HTTP/1.1\r\n", new_command.file_path);
            sprintf(HOST_str, "Host: %s\r\n",new_command.host_name);
            char *raw_request[6] = {
                GET_str,
                HOST_str,
                "Connection: keep-alive\r\n",
                "User-Agent: Mozilla/5.0\r\n",
                "Accept: text/html\r\n",
                "\r\n"
            };

            // sendMessageThroughSocet(client_sock, raw_request, 6);
            
            char **response_buff;
            int response_length = receiveMessageFromSocket(client_sock, response_buff);
            for(int i = 0; i < response_length ; i++){
                printf("%s",response_buff[i]);
            }
        }
        printf("closing connection\n");
        close(client_sock);
    }
    
    exit(0);
}