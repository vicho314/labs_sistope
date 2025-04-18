#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>

volatile sig_atomic_t token_recibido = 0;
volatile pid_t p_aux = 0;
volatile pid_t p_ant = 0;
volatile pid_t p_post = 0;

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
	if( (p_ant == p_post) && (p_post != getpid())){
		enviar(getppid(),p_ant,SIGUSR2);//enviar ganador
		enviar(getppid(),GANADOR,SIGUSR1);
	}
	else{
		enviar(p_ant,-1,SIGUSR1);
		enviar(p_post,-2,SIGUSR1);
		enviar(p_ant,p_post,SIGUSR2);
		enviar(p_post,p_ant,SIGUSR2);
		enviar(getppid(),p_ant,SIGUSR2);
		enviar(getppid(),NO_GANADOR,SIGUSR1);
	}
	exit(0);
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
	p_aux = (pid_t) si->si_value.sival_int;	//
}

/* Entradas:
 * Salidas:
 * Descripcion: Maneja la senal que recibe el token de algun proceso
*/
void extraer_token(int sig, siginfo_t * si, void *context){
	// Extrae el token enviado a traves del campo sival_int de la union
	// sigval.
	token_recibido = si->si_value.sival_int;
	printf("\nProceso %d; Token rec: %d ; ", getpid(), token_recibido);
	enviar(p_ant,RECIBIDO,SIGUSR2);
	token = desafio_random(token,M);
	printf("Token resultante: %d ;", token);
	if(token >= 0){
		enviar(p_post,token,SIGUSR1);
		recibir(SIGUSR2,extraer_pid);//esperar confirmacion
	}
	else{
		printf("(P. eliminado)");
		eliminar_proceso(p_ant,p_post);
	}
}



/* Entradas:
 * Salidas:
 * Descripcion: Asigna la accion a una señal
*/
void asignar_signal(int sig, sigaction_f accion){
	struct sigaction sa;
	//sigset_t mask, oldmask;

	// Configuracion del manejador para SIGUSR1 .
	sa.sa_flags = SA_SIGINFO;	// Permite acceder a informacion
	//extendida .
	sa.sa_sigaction = accion;	// Asigna el manejador definido

	sigemptyset(&sa.sa_mask);
	if (sigaction(sig, &sa, NULL) == -1) {
		perror("sigaction");
		exit(EXIT_FAILURE);
	}
}

/* Entradas:
 * Salidas:
 * Descripcion: Bloquea una señal
*/
sigset_t bloquear(int sig,sigset_t * mask, sigset_t * oldmask){
	// Bloquea SIGUSR1 para evitar su entrega prematura .
	sigemptyset(mask);
	sigaddset(mask, sig);
	if (sigprocmask(SIG_BLOCK, mask, oldmask) < 0) {
		perror("sigprocmask");
		exit(EXIT_FAILURE);
	}
	//*new_mask = mask; 
	return *oldmask; //para volver a activar

}
/* Entradas:
 * Salidas:
 * Descripcion: Desbloquea una señal
*/
void desbloquear(int sig){
	sigset_t mask;
	sigemptyset(&mask);
	sigaddset(&mask, sig);
	sigprocmask(SIG_UNBLOCK, &mask, NULL);
}

/* Entradas:
 * Salidas:
 * Descripcion: Recibe la señal y la desbloquea
*/
//typedef void (*sigaction_f)(int, siginfo_t *, void *);
pid_t recibir(int sig, sigaction_f accion,int block){
	sigset_t oldmask,mask;
	asignar_signal(sig,accion,&mask);
	oldmask = bloquear(sig,&mask,&oldmask);
	sigsuspend(&oldmask);
	if(block == 0){
		sigprocmask (SIG_UNBLOCK, &mask, NULL);
	}
	return p_aux;
}

/* Entradas:
 * Salidas:
 * Descripcion: Crea un proceso hijo, y enlaza mediante señales
 * al proceso anterior y proceso posterior. El proceso queda esperando
 * a la señal del token para ejecutar el desafio random.
*/
pid_t crear_hijo(pid_t inicio,int num_id, int M){
	//pid_t p_post, p_ant;
	sigset_t mask, oldmask;
	bloquear(SIGUSR2,mask,oldmask);
	pid_t pid = fork();
	int enlazado=0;
	if(pid == 0){
	    while(1){
		if(enlanzado == 0){
			if(num_id != 0){
				p_ant = inicio;
				if(num_id == -1){//caso final
					p_post = *final;
					enviar(p_post,getpid(),SIGUSR2);
				}
				else{
					recibir(SIGUSR2,extraer_pid,1);
					p_post = p_aux;
				} 
				//avisar al anterior
				enviar(p_ant,getpid(),SIGUSR2);
				//enlace(p_post);
			}
			else{//si es el primero
				recibir(SIGUSR2,extraer_pid,1);
				p_ant = p_aux;
				enviar(p_ant,getpid(),SIGUSR2);
				recibir(SIGUSR2,extraer_pid,1);
				p_post = p_aux;
			}
			enlazado=1;
		}
		/*
		else if (token_recibido != -1 && token_recibido != -2){ //reenlazar
			recibir(SIGUSR2,extraer_pid);
			p_ant = p_aux;
			enviar(p_ant,getpid(),SIGUSR2);
			recibir(SIGUSR2,extraer_pid);
			p_post = p_aux;
		}*/	
		//bloquear(SIGUSR2,&mask,&oldmask);
		recibir(SIGUSR1,extraer_token,1);
		if(token_recibido == -1){
			recibir(SIGUSR2,extraer_pid,1);
			p_post = p_aux;
			continue; //reiniciar
		}
		if(token_recibido == -2){
			recibir(SIGUSR2,extraer_pid,1);
			p_ant = p_aux;
			continue; //reiniciar
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
 * los deja esperando. Asigna el lider en cada iteracion, y recibe el ganador.
*/
//pid_t prometeo
pid_t crear_desafio(int P, int token_i, int M){
	int i,j,respuesta;
	sigset_t mask, mask2, oldmask, ublock1, ublock2;
	pid_t pid, primero;
	pid=0;
	bloquear(SIGUSR1,&mask, &mask2);
	ublock2 = mask;
	sigaddset(&ublock2,SIGUSR2);
	bloquear(SIGUSR2,&mask2,NULL);
	ublock1 = mask2;
	sigaddset(SIGUSR2,&ublock1);
	for(i=0; i < P; ++i){
		if( (i + 1) == P){
			pid = crear_hijo(pid,-1,M);
		}
		else{
			pid = crear_hijo(pid,i,M);
			if(i==0){
				primero=pid;
			}
		}
	}
	printf("Todos los hijos han sido creados.\n");
	enviar(pid,primero,SIGUSR2);
	j=0;
	respuesta=0;
	//Esperar al ganador
	while(1){
		printf("");
		if(respuesta != GANADOR){
			enviar(pid,token_i,SIGUSR1);//quitar deadlock
			//recibir(SIGUSR2,extraer_pid,-1);
			asignar_signal(SIGUSR2,extraer_pid);
			sigsuspend(&ublock2);
			pid = p_aux;//recibir lider
			//recibir(SIGUSR1,extraer_token,-1);
			asignar_signal(SIGUSR1,extraer_token);
			sigsuspend(&ublock1);
			respuesta = token_recibido;
		}
		else{
			printf("El ganador es %d.\n", pid);
			break;
		}
	}
	return pid;
}
