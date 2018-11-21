#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>

#define MAXPENDING 5 /* Maximum outstanding connection requests */
#define MAXSIZE 255 /* Maximum Buffer size */

struct Command
{
    short is_post; /* 0: GET, otherwise: POST */
    char* file_path;
    char* host_name;
    unsigned short port;
};

void DieWithError(char *errorMessage); /* Error handling function */
int CreateTCPServerSocket(unsigned short port); /* Create TCP server socket */
int AcceptTCPConnection(int servSock); /* Accept TCP connection request */
struct sockaddr_in ConnectToTCPServer(int clientSock, char *servlP, unsigned short servPort);
void sendMessageThroughSocet(int sock, char** message, int len);
int receiveMessageFromSocket(int sock, char **buff);

FILE *openFile(char* localPath);
struct Command parse_command(char *command);
int split_string(char *string, const char *delm,char **strings);