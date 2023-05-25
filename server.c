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

int get_filename(char *data, char *filename){
    int i = 0;
    for(i = 0; i < strlen(data); i++){
        if ( data[i] != ':') {
            filename[i] = data[i];
        } else {
            filename[i] = '\0';
            break;
        }
    }
    return i + 1;
}

int receive_message(int sock, int *is_file_already, int *close_connection, char *filename){
    int c_data;
    char buff[BUFSZ] = {0};

    int is_get_filename = 0;
    char path[100] = "./servidor/"; // LEMBRAR DE APAGAR ESSA PARTE
    FILE *fp;
    while(1){
        c_data = recv(sock, buff, BUFSZ, 0);
        if (is_get_filename == 0) {
            int size_filename = get_filename(buff, filename);
            is_get_filename = 1;
            strcat(path, filename);
            if (access(path, F_OK) == 0) {
                *is_file_already = 1;
            }
            fp = fopen(path, "w+");
            strcpy(buff, buff + size_filename);
        }

        if (c_data <= 0){
            break;
        }
        if (strstr(buff, "\\end")){
            buff[strlen(buff) - 4] = '\0';  
        }
        
        fprintf(fp, "%s", buff);
        bzero(buff, BUFSZ);
    }
    fclose(fp);
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

        int is_file_already = 0;
        int close_connection = 0;
        char filename[BUFSIZ];
        int r_msg = receive_message(csock, &is_file_already, &close_connection, filename);
        if(r_msg == -1){
            printf("error receiving file [%s]", filename);
        }
        if (r_msg == 0 && is_file_already == 0){
            printf("file [%s] received", filename);
        }
        if(r_msg == 0 && is_file_already == 1){
            printf("file [%s] overwritten", filename);
        }
        
        close(csock);
        printf("connection closed");
    }

    exit(EXIT_SUCCESS);
}