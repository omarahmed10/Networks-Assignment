#include "TCPFunctions.h"

#define MAXSIZE 255 /* Maximum Buffer size */

FILE *openFile(char* localPath){
    char cwd[MAXSIZE];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        FILE *fp;
        fp = fopen(strcat(cwd, localPath), "r");
        if(fp == NULL) {
            perror("Error opening file");
            return NULL;
        }
        return fp;
    } else {
        perror("getcwd() error");
        return NULL;
    }
}

struct Command parse_command(char *command){
    struct Command __command__;
    memset(&__command__, 0, sizeof(__command__)); /* Zero out structure */
    char **strings = (char **)malloc(255*sizeof(char *));
    int string_count = split_string(command, " ",strings);
    __command__.is_post = strcmp(strings[0], "GET");
    __command__.file_path = strings[1];
    __command__.host_name = strings[2];
    __command__.port = string_count > 3 ? atoi(strings[3]) : 80;
    free(strings);
    return __command__;
}

int split_string(char *string, const char *delm,char **strings){
    int size = 2;
    int i = 0;
    /* get the first token */
    strings[i] = strtok(string, delm);
    /* walk through other strings */
    while( strings[i] != NULL ) {
        i++;
        strings[i] = strtok(NULL, delm);
        // if (i == size-1){
        //     size *= 2;
        //     strings = (char **)realloc(strings,size*sizeof(char *));
        // }
    }
    return i;
}