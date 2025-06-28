#include "common.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <pthread.h>

#define CLIENTS 10
#define BUFSZ 1024

void usage(int argc, char **argv){
    printf("usage: %s <v4|v6> <server port>\n", argv[0]);
    printf("example: %s v4 51511\n", argv[0]);
    exit(EXIT_FAILURE);
}

int startConnection(int argc, char **argv, char *straddr)
{

    if(argc < 3){
        usage(argc, argv);
    }

    struct sockaddr_storage storage;
    if(0 != server_sockaddr_init(argv[1], argv[2], &storage)){
        usage(argc, argv);
    }

    int temp_sock;
    temp_sock = socket(storage.ss_family, SOCK_STREAM, 0);
    if(temp_sock == -1){
        logexit("socket");
    }

    int enable = 1;
    if(0 != setsockopt(temp_sock, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int))){
        logexit("setsockopt");
    }

    struct sockaddr *addr = (struct sockaddr *)(&storage);
    if(0 != bind(temp_sock, addr, sizeof(storage))){
        logexit("bind");
    }
    
    printf("Servidor iniciado em modo IP%s na porta %s. Aguardando conexão...\n", argv[1],argv[2]);

    if(0 != listen(temp_sock, CLIENTS)){
        logexit("listen");
    }

    return temp_sock;
}

void * doStuff(void* p_client_sock){
    int client_sock = *((int*) p_client_sock);
    free(p_client_sock);
    
    size_t count = 0;

    char Game[10];
    strcpy(Game,"Ola Mundo");

    while (1){
        count = send(client_sock, &Game, sizeof(Game), 0);
        if(count != sizeof(Game)){
            logexit("send");
        }

        count = recv(client_sock,&Game,sizeof(Game),0); //Recebe Mensagens
        if(count != sizeof(Game)){
            logexit("recv");
        }

        printf("%s\n",Game);
    } 

    return NULL;
}

void * cronometro(void * time_to_wait){
    float cronometro = *((float*) time_to_wait);

    for(int i = 0; i < 100; i++){
        usleep(100000);
        cronometro+= 0.1;
        printf("na função cronometro: %.2f\n",cronometro);
        *(float*) time_to_wait = cronometro;
    }

    *(int*) time_to_wait = cronometro;
}

int main(int argc, char **argv){

    int server_sock;
    char caddrstr[BUFSZ];

    pthread_mutex_t timing = PTHREAD_MUTEX_INITIALIZER;
    float *time_to_wait = malloc(sizeof(float));
    *time_to_wait = 0;
    
    server_sock = startConnection(argc, argv, caddrstr);//Conecta ao cliente

    int N = 0;

    while(N <= CLIENTS && *time_to_wait < 10){

        struct sockaddr_storage cstorage;
        struct sockaddr *caddr = (struct sockaddr *)(&cstorage);
        socklen_t caddrlen = sizeof(cstorage);

        int client_sock = accept(server_sock, caddr, &caddrlen);
        if (client_sock == -1)
        {
            logexit("accept");
        }
        printf("Cliente conectado.\n");
        N++;

        if(N == 1){
            pthread_t temporizador;
            pthread_create(&temporizador,NULL,cronometro,time_to_wait);
        }
        printf("cronometro: %.2f\n", *time_to_wait);

        char traddr[BUFSZ];
        addrtostr(caddr, caddrstr, BUFSZ);

        pthread_t t;
        int *pclient = malloc(sizeof(int));
        *pclient = client_sock;
        pthread_create(&t,NULL,doStuff,pclient);
    }

    printf("Free\n");

    free(time_to_wait);
}