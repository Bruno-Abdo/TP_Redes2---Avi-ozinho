#pragma once

#include <stdlib.h>

#include <arpa/inet.h>

void logexit(const char *msg);

int addrparse(const char *addrstr, const char *portstr,
              struct sockaddr_storage *storage);

void addrtostr(const struct sockaddr *addr, char *str, size_t strsize);

int server_sockaddr_init(const char *proto, const char *portstr,
                         struct sockaddr_storage *storage);
                         
#define STR_LEN 11
#define BUFSZ 1024
typedef struct{
int32_t player_id;
float value;
char type[STR_LEN];
float player_profit;
float house_profit;
}aviator_msg;

typedef struct {  
    int socket;
    int connected;
    float time_to_wait;
    char events[10];
    int dest_msg;
    int client_sock;
    float V;
    int N;
    float Me;
    float M;
} newConection;
