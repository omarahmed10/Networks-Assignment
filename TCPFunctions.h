#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>

typedef int bool;
#define true 1
#define false 0
#define MAXPENDING 5 /* Maximum outstanding connection requests */
#define MAXSIZE 255 /* Maximum Buffer size */

struct Command
{
    short is_post; /* 0: GET, otherwise: POST */
    char* file_path;
    char* host_name;
    unsigned short port;
};
struct Req_Head
{
    char * file_path;
    bool is_GET;
};
extern short operation;
void DieWithError(char *errorMessage); /* Error handling function */
int CreateTCPServerSocket(unsigned short port); /* Create TCP server socket */
int AcceptTCPConnection(int servSock); /* Accept TCP connection request */
struct sockaddr_in ConnectToTCPServer(int clientSock, char *servlP, unsigned short servPort);
void sendMessageThroughSocet(int sock, char* message, int len);
void receiveResponseFromSocket(int sock, FILE *outputFile); // for client in case of GET.
char *receiveRequestHeaderFromSocket(int sock);
void receiveResponseBodyFromSocket(int sock, FILE *outputFile); // for server in case of POST.

FILE *openFile(char* localPath, char *operation);
struct Command parse_command(char *command);
int split_string(char *string, const char *delm,char **strings);
char *str_find_next(char *string, char *key, char *key2);
bool str_find_empty_line(char *string);
short str_exist(char *string, char *key, char *key2);