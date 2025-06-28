#include "common.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define BUFSZ 1024

void usage(int argc, char **argv) // Verifica a conexão do servidor
{
	printf("usage: %s <server IP> <server port>\n", argv[0]);
	printf("example: %s 127.0.0.1 51511\n", argv[0]);
	exit(EXIT_FAILURE);
}

int startConnection(int argc, char **argv, int s) // Implementa a conexão com o servidor socket() e connect()
{
	if (argc < 3)
	{
		usage(argc, argv);
	}

	struct sockaddr_storage storage;
	if (0 != addrparse(argv[1], argv[2], &storage))
	{
		usage(argc, argv);
	}

	s = socket(storage.ss_family, SOCK_STREAM, 0);
	if (s == -1)
	{
		logexit("socket");
	}
	struct sockaddr *addr = (struct sockaddr *)(&storage);
	if (0 != connect(s, addr, sizeof(storage)))
	{
		logexit("connect");
	}
	printf("Conectado ao servidor.\n");

	return s;
}

int main(int argc, char **argv)
{
	int s;				// Inicialização do Socket
	char Game[10];
	int num = 0;
	
	s = startConnection(argc, argv, s); // Faz socket(), bind(), listen() e accept()

	while(1){

		recv(s, &Game, sizeof(Game), 0);

		printf("%s\n",Game);

		strcpy(Game,"Entao ok");
		send(s,&Game, sizeof(Game),0);

		sleep(1);
		num++;
		printf("%d\n",num);

	}
}