#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>

#define SOY_POST 0
#define SOY_ANT 1
#define RECIBIDO 2

volatile sig_atomic_t token_recibido = 0;
volatile pid_t pid_aux_global;
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
void enviar(pid_t pid, int token, int sig){
	value.sival_int = token; // Asigna el token ( valor de ejemplo)
	if (sigqueue(pid, sig, value) == -1) {
		perror("sigqueue");
		exit(EXIT_FAILURE);
	}
}


/* Entradas:
 * Salidas:
 * Descripcion: Reenlaza los procesos correspondientes al eliminar uno.
*/
void eliminar_proceso(pid_t p_ant, pid_t p_post){
	enviar(p_ant,-1,SIGUSR1);
	enviar(p_post,-2,SIGUSR1);
	enviar(p_ost,p_ant,SIGUSR2);
	enviar(p_ant,p_post,SIGUSR2);
}

/* Entradas:
 * Salidas:
 * Descripcion: Maneja la senal que recibe el pid de algun proceso (siguiente
 * o anterior)
*/
void extraer_pid(int sig, siginfo_t * si, void *context)
{
	// Extrae el token enviado a traves del campo sival_int de la union
	// sigval.
	//token_recibido = si->si_value.sival_int;
	
	//printf(" Proceso %d recibio el token : %d\n", getpid(), token);
	pid_aux = (pid_t) si->si_value.sival_int;	//
}

void extraer_token(int sig, siginfo_t * si, void *context){
	// Extrae el token enviado a traves del campo sival_int de la union
	// sigval.
	token_recibido = si->si_value.sival_int;
	printf("Proceso %d recibio el token : %d\n", getpid(), token_recibido);
}

/* Entradas:
 * Salidas:
 * Descripcion: Recibe el pid del siguiente proceso
*/
typedef void (*sigaction_f)(int, siginfo_t *, void *);
pid_t recibir(int sig,sigaction_f accion ){
	sigset_t oldmask;
	oldmask = asignar_signal(sig,accion);
	sigsuspend(&oldmask);
	return p_aux;
}

/* Entradas:
 * Salidas:
 * Descripcion: Asigna las mascaras de una señal, y la bloquea
*/

sigset_t asignar_signal(int sig, sigaction_f accion){
	struct sigaction sa;
	sigset_t mask, oldmask;

	// Configuracion del manejador para SIGUSR1 .
	sa.sa_flags = SA_SIGINFO;	// Permite acceder a informacion
	//extendida .
	sa.sa_sigaction = accion;	// Asigna el manejador definido

	sigemptyset(&sa.sa_mask);
	if (sigaction(sig, &sa, NULL) == -1) {
		perror("sigaction");
		exit(EXIT_FAILURE);
	}
	// Bloquea SIGUSR1 para evitar su entrega prematura .
	sigemptyset(&mask);
	sigaddset(&mask, SIGUSR1);
	if (sigprocmask(SIG_BLOCK, &mask, &oldmask) < 0) {
		perror("sigprocmask");
		exit(EXIT_FAILURE);
	}
	return oldmask; //para volver a activar
}

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
pid_t crear_hijo(pid_t inicio, pid_t * final,int num_id){
	pid_t p_post, p_ant;
	pid_t pid = fork();
	int enlazado=0;
	if(pid == 0){
	    while(1){
		if(enlanzado == 0){
			if(num_id != 0){
				p_ant = inicio;
				if(final != NULL){//caso final
					p_post = *final;
					enviar(p_post,getpid(),SIGUSR2);
				}
				else{
					recibir(SIGUSR2,extraer_pid);
					p_post = p_aux;
				} 
				//avisar al anterior
				enviar(p_ant,getpid(),SIGUSR2);
				//enlace(p_post);
			}
			else{//si es el primero
				recibir(SIGUSR2,extraer_pid);
				p_ant = p_aux;
				enviar(p_ant,getpid(),SIGUSR2);
				recibir(SIGUSR2,extraer_pid);
				p_post = p_aux;
			}
			enlazado=1;
		}
		else{ //reenlazar
			recibir(SIGUSR2,extraer_pid);
			p_ant = p_aux;
			enviar(p_ant,getpid(),SIGUSR2);
			recibir(SIGUSR2,extraer_pid);
			p_post = p_aux;
		}
		recibir(SIGUSR1,extraer_token);
		if(token_recibido == -1){
			continue; //reiniciar
		}
		token = token_recibido;
		enviar(p_ant,RECIBIDO,SIGUSR2);
		token = desafio_random(token);
		if(token >= 0){
			enviar_token(p_post,token,SIGUSR1);
			recibir(SIGUSR2,extraer_pid);//esperar confirmacion
		}
		else{
			eliminar_proceso(p_ant,p_post);
		}
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
