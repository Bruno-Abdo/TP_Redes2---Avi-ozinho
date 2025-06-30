#include "common.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>


#define BUFSZ 1024

int startConnection(int argc, char **argv, int s) // Implementa a conexão com o servidor socket() e connect()
{		
	if(argc < 4){
		printf("Error: Invalid Number of Arguments\n");
		exit(EXIT_FAILURE);
	}

	if(strcmp(argv[3],"-nick")){
			printf("Error: Expected'-nick' argument\n");
			exit(EXIT_FAILURE);
	}

	if(strlen(argv[4]) >= 13){
		printf("Error: Nickname too long (max 13)\n");
		exit(EXIT_FAILURE);
	}

	struct sockaddr_storage storage;
	addrparse(argv[1], argv[2], &storage);

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

void * clientCommand(void * check){

	char entrada;
	while(1){
		scanf("%c",entrada);
		if(entrada == 'c' || entrada == 'C'){
			*(int*)check = 0;
			break;
		} else{
			printf("Error: Invalid command\n");
		}
	}

	pthread_exit(NULL);
}

int main(int argc, char **argv){
	int s;				// Inicialização do Socket
	int num = 0;
	aviator_msg Game;
	char input[6];
	float aposta = 0;
	newConection *client = malloc(sizeof(newConection));

	char nick[13];

	s = startConnection(argc, argv, s); // Faz socket(), bind(), listen() e accept()

	strcpy(nick,argv[4]);

	strcpy(Game.type,"start");
	pthread_t bets;

	int* check = malloc(sizeof(int));
	
	*check = 1;

	int beted = 0;
	int cashed = 0;

	while(1){

		recv(s,&Game,sizeof(Game),0);

		printf("Client id: %d\n",Game.player_id);

		if(!*check){
			strcpy(Game.type,"cashout");
		}

		if(!strcmp(Game.type,"start")){
			printf("Rodada Aberta! Digite o valor da aposta ou digite [Q] para sair (%.2f segunsdos restantes): ",Game.value);
			fgets(input,sizeof(input),stdin);
			if (strlen(input) == 2 && (input[0] == 'Q' || input[0] == 'q')){
        		printf("Saindo do programa.\n");
				break;
    		}else if((int *)input[0] < 48 || (int *)input[0] > 58){
				printf("Error: Invalid command\n");
			}else{
				aposta = strtof(input,NULL);
				if(aposta <= 0){
					printf("Error: Invalid bet value\n");
				}
				strcpy(Game.type, "bet");
				Game.value = aposta;
				send(s,&Game, sizeof(Game),0);
				beted = 1;
				printf("Aposta recebida: %.2f\n",aposta);
				strcpy(Game.type, "wainting");
			}
		}

		if(!strcmp(Game.type,"closed")){
			if(beted){
				printf("Apostas encerradas! Não é mais possível apostar nesta rodada. Digite [C] para sacar.");
				pthread_create(&bets,NULL,clientCommand,check);
			}else{
				printf("Apostas encerradas! Não é mais possível apostar nesta rodada.");
			}
				
		}

		if(!strcmp(Game.type,"multiplier")){
			printf("Multiplicador atual: %.2fx\n",Game.value);
		}

		if(!strcmp(Game.type,"cashout")){

			strcpy(Game.type, "cashout");
			send(s,&Game, sizeof(Game),0);
			cashed = 1;
		}

		if(!strcmp(Game.type,"payout")){

			printf("Você sacou em %.2fx e ganhou R$ %.2f!",Game.value,Game.player_profit);
		}

		if(!strcmp(Game.type,"explode")){
			if(cashed){
				printf("Avinhãozinho explodiu em : %.2f",Game.value);
			}else{
				printf("Avinhãozinho explodiu em : %.2f",Game.value);
				printf("Você perdeu R$ %.2f. Tente novamente na próxima rodada! Aviãozinho tá pagando :)", aposta);
			}
		}

		if(!strcmp(Game.type,"profit")){
			if(cashed){
				printf("Profit da casa : R$ %.2f",Game.house_profit);
			}else{
				printf("Profit atual: R$ -%.2f",Game.player_profit);
				printf("Profit da casa : R$ %.2f",Game.house_profit);
			}

		}

	}//while

	free(check);
	close(s);
}//main