#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>

#define SOY_POST 0
#define SOY_ANT 1
#define RECIBIDO 2
#define GANADOR -3

extern volatile sig_atomic_t token_recibido;
extern volatile pid_t p_aux;
extern volatile pid_t p_ant;
extern volatile pid_t p_post;

typedef void (*sigaction_f)(int, siginfo_t *, void *);
int desafio_random(int token, int M);
void enviar(pid_t pid, int token, int sig);
void eliminar_proceso(pid_t p_ant, pid_t p_post);
void extraer_pid(int sig, siginfo_t * si, void *context);
void extraer_token(int sig, siginfo_t * si, void *context);
void asignar_signal(int sig, sigaction_f accion);
sigset_t bloquear(int sig,sigset_t * mask, sigset_t * oldmask);
void desbloquear(int sig);
pid_t recibir(int sig, sigaction_f accion,int block);
pid_t crear_hijo(pid_t inicio,int num_id, int M);
pid_t crear_desafio(int P, int token_i, int M);
