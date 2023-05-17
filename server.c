#include "common.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>

#define BUFSZ 500

void usage(int argc, char **argv){
    printf("usage: %s <v4|v6> <server port>\n", argv[0]);
    printf("exemplo: %s v4 51511", argv[0]);
    exit(EXIT_FAILURE);
}

int write_file(int sock){
    FILE *fp;
    int c_data;
    char *filename = "fp_recebido.txt";
    char buff[BUFSZ];

    fp = fopen(filename, "w");
    if(fp == NULL){
        return -1;
    }

    while(1){
        c_data = recv(sock, buff, BUFSZ, 0);
        if (c_data <= 0){
            fclose(fp);
            break;
        }
        printf("%s\n", buff);
        fprintf(fp, "%s", buff);
        bzero(buff, BUFSZ);
    }
    return 0;
}


int main(int argc, char **argv){
    if (argc < 3){
        usage(argc, argv);
    }

    struct sockaddr_storage storage;
    if(0 != server_sockaddr_init(argv[1], argv[2], &storage)){
        usage(argc, argv);
    }

    int s;
    s = socket(storage.ss_family, SOCK_STREAM, 0);
    if (s == -1){
        logexit("socket");
    }

    int enable = 1;
    if ( 0  != setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int))){
        logexit("setsockopt");
    }

    struct sockaddr *addr = (struct sockaddr *)(&storage); // casting
    if (0 != bind(s, addr, sizeof(storage))){
        logexit("bind");
    }

    if (0 != listen(s, 10)){
        logexit("listen");
    }

    char addrstr[BUFSZ];
    addrtostr(addr, addrstr, BUFSZ);
    printf("bound to %s, waiting connections\n", addrstr);

    while(1){
        struct sockaddr_storage cstorage;
        struct sockaddr *caddr = (struct sockaddr *)(&cstorage);
        socklen_t caddrlen = sizeof(cstorage);
        
        int csock = accept(s, caddr, &caddrlen);
        if (csock == -1){
            logexit("accept");
        }

        char caddrstr[BUFSZ];
        addrtostr(caddr, caddrstr, BUFSZ);
        printf("[log] connection from %s\n", caddrstr);
        
        // char buf[BUFSZ];
        // memset(buf, 0, BUFSIZ);
        // size_t count = recv(csock, buf, BUFSZ, 0);
        // Implementar a leituar parcial dos dados, igual no envio do cliente
        char response[BUFSZ];
        if(write_file(csock) == -1){
            strcpy(response, "error receiving file [nomearquivo]");
        }else{
            strcpy(response, "file [nomearquivo] received");
        }
        size_t count = send(csock, response, strlen(response)+1, 0);
        if (count != strlen(response)+1){
            logexit("send");
        }
        printf("[msg] %s, %d bytes: %s\n", caddrstr, (int)count, response);

        // sprintf(buf, "remote endpoint: %.500s\n", caddrstr);
        // count = send(csock, buf, strlen(buf)+1, 0);
        // if (count != strlen(buf)+1){
        //     logexit("send");
        // }
        close(csock);
    }

    exit(EXIT_SUCCESS);
}