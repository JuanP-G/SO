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

static void on_alarm(int signal_number)
{
    (void)signal_number;
    kill(child_pid, SIGKILL);   /* se acabó el tiempo: matar al hijo */
}

int main(int argc, char **argv)
{
    if (argc < 2) {
        fprintf(stderr, "Uso: %s comando [argumentos...]\n", argv[0]);
        return 1;
    }

    child_pid = fork();
    if (child_pid == -1) {
        perror("fork");
        return 1;
    }
    if (child_pid == 0) {                       /* hijo: cambia de ejecutable */
        execvp(argv[1], &argv[1]);
        perror(argv[1]);                        /* solo si execvp falla */
        _exit(127);
    }

    /* padre: manejador de SIGALRM */
    struct sigaction on_alarm_action;
    memset(&on_alarm_action, 0, sizeof(on_alarm_action));
    on_alarm_action.sa_handler = on_alarm;
    sigemptyset(&on_alarm_action.sa_mask);
    if (sigaction(SIGALRM, &on_alarm_action, NULL) == -1) {
        perror("sigaction SIGALRM");
        return 1;
    }

    /* padre: ignorar SIGINT (Ctrl+C). Se hace DESPUÉS del fork: una señal ignorada se
     * hereda a través de exec, y el hijo no debe ignorarla. */
    struct sigaction ignore_action;
    memset(&ignore_action, 0, sizeof(ignore_action));
    ignore_action.sa_handler = SIG_IGN;
    sigemptyset(&ignore_action.sa_mask);
    sigaction(SIGINT, &ignore_action, NULL);

    alarm(TIMEOUT_SECONDS);

    int status;
    while (waitpid(child_pid, &status, 0) == -1) {
        if (errno != EINTR) {                   /* la alarma interrumpe waitpid: reintentar */
            perror("waitpid");
            return 1;
        }
    }

    if (WIFEXITED(status))
        printf("El hijo terminó normalmente con código %d\n", WEXITSTATUS(status));
    else if (WIFSIGNALED(status))
        printf("El hijo terminó por la señal %d (%s)\n",
               WTERMSIG(status), strsignal(WTERMSIG(status)));

    return 0;
}
