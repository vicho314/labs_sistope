#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>
#include "funciones.h"


int main(int argc, char ** argv){
	pid_t ganador;
	int num_hijos = 2;
	int token_inicial = 10;
	int M = 10;

	pid = crear_desafio(num_hijos,token_inicial,M);
}
