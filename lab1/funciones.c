#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>

volatile sig_atomic_t token_recibido = 0;

/*
* Manejador de senales que procesa la senal SIGUSR1 .
* Se utiliza SA_SIGINFO para acceder a la informacion adicional enviada con
senal.
*/
void manejador(int sig, siginfo_t * si, void *context)
{
	// Extrae el token enviado a traves del campo sival_int de la union
	// sigval.
	int token = si->si_value.sival_int;
	printf(" Proceso %d recibio el token : %d\n", getpid(), token);
	token_recibido = 1;	// Indica que la senal ha sido recibida .
}

/* Entradas:
 * Salidas:
 * Descripcion: Ejecuta el desafio random, retorna el token resultante
*/
int desafio_random(int token, int M){
	//return token - (int)(drand48() * M);
	return token - (rand() % M); 
}

/* Entradas:
 * Salidas:
 * Descripcion: Elimina el proceso y se notifica a los demás 
 * procesos su salida.
*/
pid_t eliminar_proceso(pid_t p_ant, pid_t p_post){
	notificar_anterior(p_post);
	notificar_posterior(p_ant);
	salir();
}

/* Entradas:
 * Salidas:
 * Descripcion: Envia el token al siguiente proceso mediante una señal.
*/

/* Entradas:
 * Salidas:
 * Descripcion: Reenlaza los procesos correspondientes al eliminar uno.
*/

/* Entradas:
 * Salidas:
 * Descripcion: Elige un lider y reinicializa el token.
*/

/* Entradas:
 * Salidas:
 * Descripcion: Crea un proceso hijo, y enlaza mediante señales
 * al proceso anterior y proceso posterior. El proceso queda esperando
 * a la señal del token para ejecutar el desafio random.
*/
pid_t crear_hijo(pid_t p_ant, int num_id, int M){
	pid_t p_post;
	pid_t pid = fork();
	int enlazado=0;
	if(pid == 0){
		if(enlanzado == 0){
			enlace_ant(p_ant);
			p_post = recibir_post();
			enlace_post(p_post);
		}
		else{
			reenlazar();
			esperar_token();
		}
		token = recibir_token();
		token = desafio_random(token);
		if(token >= 0){
			enviar_token(token,p_post);
			esperar_token(s);
		}
		else{
			eliminar_proceso(p_ant,p_post);
		}
	}
	else if(pid < 0){
		//fprintf(stderr, "E1:Error al crear fork!\n");
		perror("fork");
	}
	else{
		return pid_t; //sigue con la secuencia
	}
}

/* Entradas:
 * Salidas:
 * Descripcion: Crea N procesos hijos, los enlaza entre sí y 
 * los deja esperando.
*/
