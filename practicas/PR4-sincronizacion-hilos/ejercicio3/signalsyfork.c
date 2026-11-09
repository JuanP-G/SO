#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>


#define TIMEOUT_SECONDS 5

/* programa que temporiza la ejecución de un proceso hijo */

static pid_t child_pid;    /* global: el manejador de señal no recibe parámetros propios */

void tratar_alarma(int signal_number){ 
	(void)signal_number;
	kill(child_pid, SIGTERM); /* envía la señal SIGTERM al hijo */
}

int main(int argc, char **argv)
{
	struct sigaction act; // estructura para especificar el manejador de señal
	pid_t pid; // variable para almacenar el PID del proceso hijo
	int status; // variable para almacenar el estado de salida del hijo

	if (argc < 2) {
		fprintf(stderr, "Uso: %s comando [args...]\n", argv[0]);
		return 1;
	}

	pid = fork();
	switch(pid) {
	case -1:			/* error del fork() */
		perror ("fork"); exit(-1);
	case 0:				/* hijo */
		execvp(argv[1], argv + 1);
		perror(argv[1]);			/* solo se llega aquí si execvp falla */
		_exit(127);
	default: /* padre */
		/* establece el manejador */
		child_pid = pid; /* guarda el PID del hijo para usarlo en el manejador */

		memset(&act, 0, sizeof(act)); /* inicializa la estructura a cero */
		act.sa_handler = (void *)tratar_alarma; /* función que maneja la señal */
		act.sa_flags = 0; /* ninguna acción específica */
		sigemptyset(&act.sa_mask); /* no bloquea ninguna señal mientras se ejecuta el manejador */
		sigaction(SIGALRM, &act, NULL); /* establece el manejador de la señal SIGALRM */
		alarm(TIMEOUT_SECONDS); /* activa la alarma */

		while (waitpid(pid, &status, 0) == -1) {	/* -1: falló, por el motivo que sea */
			if (errno != EINTR) {	/* ¿fue por algo que NO se arregla repitiendo? */
				perror("waitpid");
				return 1;			/* sí: salir con error */
			}
									/* no (EINTR): no hace nada y el while repite */
		}

		/* comprueba cómo terminó el hijo */
		if (WIFEXITED(status))
			printf("Finalización normal, código %d\n", WEXITSTATUS(status));
		else if (WIFSIGNALED(status))
			printf("Finalización por señal %d\n", WTERMSIG(status));

		return 0;
	}
}
