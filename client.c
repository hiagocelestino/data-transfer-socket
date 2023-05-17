#include "common.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define BUFSZ 500

void usage(int argc, char **argv){
    printf("usage: %s <server IP> <server port>\n", argv[0]);
    printf("exemplo: %s 127.0.0.1 51511\n", argv[0]);
    exit(EXIT_FAILURE);
}

int valid_file(char *name_file){
    char *dot = strrchr(name_file, '.');
    char *ext_allowed[6] = {".txt", ".c", ".cpp", ".py", ".tex", ".java"};

    for (int i = 0; i < 6; i++){
        if (dot && strcmp(dot, ext_allowed[i]) == 0 ){
            return 0;
        }
    }
    return -1;
}

FILE *get_file(char *filename){
    FILE *fp = fopen(filename, "r");
    if(fp == NULL){
        printf("[%s] do not exist\n", filename);
    }
    return fp;
}

void send_file(FILE *fp, int sock){
    char data[BUFSZ] = {0};

    while(fgets(data, BUFSZ, fp) != NULL){
        printf("%s", data);
        if(send(sock, data, sizeof(data), 0) == -1){
            printf("Error in send data.\n");
        }
        bzero(data, BUFSZ);
    }
    char response[BUFSZ];
    // recv(sock, response, BUFSZ, 0);
    printf("%s\n", response);
}

void filter_option_client(int socket, char *option, int *is_file_selected, char *filename){
    // TODO: Esta printando tudo
    option[strlen(option) - 1] = '\0';
    if (strstr(option, "select file")){
        char tr[20];
        sscanf(option, "%s %s %s", tr, tr, filename);
        if (valid_file(filename) == -1){
            printf("[%s] not valid!\n", filename);
        }else{
            *is_file_selected = 1;
            printf("[%s] selected\n", filename);
        }
    }

    if (strcmp(option, "send file") == 0){
        if (*is_file_selected == 0){
            printf("no file selected!\n");
        }else{
            FILE *fp = get_file(filename);
            rewind(fp);
            strcat(filename, "||");
            printf("%s\n", filename);
            fwrite(filename, 1, sizeof(filename), fp);
            send_file(fp, socket);
        }
    }
    
}

int main(int argc, char **argv){
    if (argc < 3){
        usage(argc, argv);
    }


    struct sockaddr_storage storage;
    if(0 != addrparse(argv[1], argv[2], &storage)){
        usage(argc, argv);
    }
    
    int s;
    s = socket(storage.ss_family, SOCK_STREAM, 0);
    if (s == -1){
        logexit("socket");
    }
    struct sockaddr *addr = (struct sockaddr *)(&storage); // casting
    if (0 != connect(s, addr, sizeof(storage))){
        logexit("connect");
    };

    char addrstr[BUFSZ];
    addrtostr(addr, addrstr, BUFSZ);

    printf("connected to %s\n", addrstr);

    char buff[BUFSZ];
    memset(buff, 0, BUFSZ);


    printf("[command]> ");
    
    int is_file_selected = 0;
    char filename[50];
    while(1){
        fgets(buff, BUFSZ-1, stdin);
        if(strstr(buff, "exit")){
            break;
        }
        filter_option_client(s, buff, &is_file_selected, filename);
    }
    close(s);

    exit(EXIT_SUCCESS);

}