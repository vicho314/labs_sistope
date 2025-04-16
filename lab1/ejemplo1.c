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

int main()
{
	struct sigaction sa;
	sigset_t mask, oldmask;

	// Configuracion del manejador para SIGUSR1 .
	sa.sa_flags = SA_SIGINFO;	// Permite acceder a informacion
	//extendida .
	sa.sa_sigaction = manejador;	// Asigna el manejador definido

	sigemptyset(&sa.sa_mask);
	if (sigaction(SIGUSR1, &sa, NULL) == -1) {
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

	pid_t pid = fork();
	if (pid == -1) {
		perror("fork");
		exit(EXIT_FAILURE);
	} else if (pid == 0) {
		// Proceso hijo : espera la senal de manera segura usando
		//sigsuspend .
		while (!token_recibido) {
			// sigsuspend reemplaza temporalmente la mascara
			//actual por ’oldmask ’,
			// permitiendo la recepcion de SIGUSR1 .
			sigsuspend(&oldmask);
		}
		exit(0);
	} else {
		// Proceso padre : prepara y envıa la senal con el token .
		union sigval value;
		value.sival_int = 42;	// Asigna el token ( valor de ejemplo)
		if (sigqueue(pid, SIGUSR1, value) == -1) {
			perror("sigqueue");
			exit(EXIT_FAILURE);
		}
		wait(NULL);	// Espera a que el proceso hijo termine .
	}
	return 0;
}
