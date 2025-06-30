#include "common.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <pthread.h>
#include <math.h>

#define CLIENTS 10

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

void * recv_cashOut(void * cashout){

    aviator_msg Game;
    int count = 0;
    while(1){
    count = recv(((int *)cashout)[0],&Game,sizeof(Game),0); //Recebe Mensagens
    if(count != sizeof(Game)){
        logexit("recv");
        pthread_exit(NULL);
    }
    if(!strcmp(Game.type,"cashout")){
        ((int *)cashout)[1] = 1;
    }
}

}

void * doStuff(void* server_info){
    
    aviator_msg Game;
    size_t count = 0;

    int client_sock = ((newConection*)server_info)->client_sock;
    Game.player_id = ((newConection*)server_info)->N;
    float aposta = 0;
    
    while (1){
        if(strcmp(((newConection*)server_info)->events,"start") == 0){
            strcpy(((newConection*)server_info)->events,"waiting");
            Game.house_profit = 0;
            Game.player_profit = 0;
            strcpy(Game.type,"start");
            Game.value = ((newConection*)server_info)->time_to_wait;

            count = send(client_sock, &Game, sizeof(Game), 0);
            if(count != sizeof(Game)){
                logexit("send");
            }

            count = recv(client_sock,&Game,sizeof(Game),0); //Recebe Mensagens
            if(count != sizeof(Game)){
                logexit("recv");
                pthread_exit(NULL);
            }
        }

        if(!strcmp(Game.type,"bet")){
            aposta = Game.value;
            ((newConection*)server_info)->V += aposta;
            //break;
            printf("event=bet | id=%d |bet=%.2f | N=%d | V=%.2f\n",Game.player_id,aposta,((newConection*)server_info)->N,((newConection*)server_info)->V);
             strcpy(Game.type,"waiting");
        }

        if(strcmp(((newConection*)server_info)->events,"closed") == 0){
            strcpy(Game.type,"closed");

            count = send(client_sock, &Game, sizeof(Game), 0);
            if(count != sizeof(Game)){
                logexit("send");
            }

            int *cashout;
            cashout = (int *)malloc(2*sizeof(int));

            cashout[0] = client_sock;
            pthread_t t;
            pthread_create(&t,NULL,recv_cashOut,server_info);
            strcpy(((newConection*)server_info)->events,"waiting");
            printf("saindo\n");
        }
    }
        //printf("%d\n",Game.player_id);

    return NULL;
}

void * cronometro(void * server_time){
    float cronometro = (*(newConection*)server_time).time_to_wait;
    ((newConection*)server_time)->time_to_wait = 0.1;
    if(cronometro == 10){
        strcpy(((newConection*)server_time)->events,"start");
        printf("event=start | id=* | N=%d\n",((newConection*)server_time)->N);
    }

    for(int i = 0; i < cronometro*10; i++){
        usleep(100000);
        ((newConection*)server_time)->time_to_wait += 0.1;
    }

    strcpy(((newConection*)server_time)->events,"close");
    printf("event=closed | id=* | N = %d | V = %.2f\n",((newConection*)server_time)->N,((newConection*)server_time)->V);
    ((newConection*)server_time)->Me = sqrtf(1+((newConection*)server_time)->N +0.01*((newConection*)server_time)->V);

    printf("%.2f\n",((newConection*)server_time)->Me);
    ((newConection*)server_time)->M = 1;

    printf("%.2f\n",((newConection*)server_time)->M);
    while(1){
        usleep(100000);
        ((newConection*)server_time)->M += 0.1;
        printf("event=multiplier | id=* | m=%.2f",((newConection*)server_time)->M);
        if(((newConection*)server_time)->M >= ((newConection*)server_time)->Me){
            break;
        }
    }

    strcpy(((newConection*)server_time)->events,"explode");

    pthread_exit(NULL);
}

int main(int argc, char **argv){

    newConection *server = malloc(sizeof(newConection));
    char caddrstr[BUFSZ];

    server->socket = startConnection(argc, argv,caddrstr);//Conecta ao cliente
    server->time_to_wait = 0;

    while(1){
    
        struct sockaddr_storage cstorage;
        struct sockaddr *caddr = (struct sockaddr *)(&cstorage);
        socklen_t caddrlen = sizeof(cstorage);

        server->client_sock = accept(server->socket, caddr, &caddrlen);
        if (server->client_sock == -1){
            logexit("accept");
        }

        server->N++;

        pthread_t t;
        pthread_create(&t,NULL,doStuff,server);

        char traddr[BUFSZ];
        addrtostr(caddr, caddrstr, BUFSZ);

        if(server->time_to_wait == 0){

            server->time_to_wait = 10;
            pthread_t temporizador;
            pthread_create(&temporizador,NULL,cronometro,server);
        }
    }
    sleep(1);  
}