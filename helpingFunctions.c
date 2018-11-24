#include "TCPFunctions.h"

#define MAXSIZE 255 /* Maximum Buffer size */

FILE *openFile(char* localPath, char *operation){
    char cwd[MAXSIZE];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        FILE *fp;
        fp = fopen(strcat(cwd, localPath), operation);
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
    int i = 0;
    /* get the first token */
    strings[i] = strtok(string, delm);
    /* walk through other strings */
    while( strings[i] != NULL ) {
        i++;
        strings[i] = strtok(NULL, delm);
    }
    return i;
}

char *str_find_next(char *string, char *key, char *key2){
    /* get the first token */
    char *token = strtok(string, " \n");
    /* walk through other strings */
    while( token != NULL ) {
        if (strcmp(token,key) == 0 || strcmp(token,key2) == 0 ){
            return strtok(NULL, " \n");
        }
        token = strtok(NULL, " \n");
    }
    return NULL;
}
short str_exist(char *string, char *key, char *key2){
    /* get the first token */
    char *token = strtok(string, " \n");
    /* walk through other strings */
    while( token != NULL ) {
        if (strcmp(token,key) == 0){
            return 1;
        }
        if (strcmp(token,key2) == 0){
            return 2;
        }
        token = strtok(NULL, " \n");
    }
    return 0;
}
bool str_find_empty_line(char *string){
    /* get the first token */
    char *token = strtok(string, "\n");
    /* walk through other strings */
    while( token != NULL ) {
        if (strlen(token) == 1 && token[0] == 13){
            return true;
        }
        token = strtok(NULL, "\n");
    }
    return false;
}