#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>     /* fork, execvp, _exit, getopt */
#include <sys/types.h>  /* pid_t */
#include <sys/wait.h>   /* wait, waitpid */

#define MAX_COMMANDS 100
#define MAX_LINE     1024

/* Crea un hijo que ejecuta argv[0] con sus argumentos. Devuelve el PID del
 * hijo SIN esperar a que termine. */
pid_t launch_command(char **argv)
{
    pid_t pid = fork();

    if (pid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }
    if (pid == 0) {                  /* hijo */
        execvp(argv[0], argv);
        perror(argv[0]);             /* solo se llega aquí si execvp falla */
        _exit(127);                  /* _exit: no vaciar los búferes de stdio heredados */
    }
    return pid;                      /* padre */
}

static void free_command(char **argv)
{
    for (int i = 0; argv[i] != NULL; i++)
        free(argv[i]);
    free(argv);
}

char **parse_command(const char *cmd, int* argc) {
    // Allocate space for the argv array (initially with space for 10 args)
    size_t argv_size = 10;
    const char *end;
    size_t arg_len; 
    int arg_count = 0;
    const char *start = cmd;
    char **argv = malloc(argv_size * sizeof(char *));

    if (argv == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    while (*start && isspace(*start)) start++; // Skip leading spaces

    while (*start) {
        // Reallocate more space if needed
        if (arg_count >= argv_size - 1) {  // Reserve space for the NULL at the end
            argv_size *= 2;
            argv = realloc(argv, argv_size * sizeof(char *));
            if (argv == NULL) {
                perror("realloc");
                exit(EXIT_FAILURE);
            }
        }

        // Find the start of the next argument
        end = start;
        while (*end && !isspace(*end)) end++;

        // Allocate space and copy the argument
        arg_len = end - start;
        argv[arg_count] = malloc(arg_len + 1);

        if (argv[arg_count] == NULL) {
            perror("malloc");
            exit(EXIT_FAILURE);
        }
        strncpy(argv[arg_count], start, arg_len);
        argv[arg_count][arg_len] = '\0';  // Null-terminate the argument
        arg_count++;

        // Move to the next argument, skipping spaces
        start = end;
        while (*start && isspace(*start)) start++;
    }

    argv[arg_count] = NULL; // Null-terminate the array

    (*argc)=arg_count; // Return argc

    return argv;
}

/* Espera a un hijo concreto y devuelve su estado en bruto (como pide el enunciado). */
static int wait_for(pid_t pid)
{
    int status;

    if (waitpid(pid, &status, 0) == -1) {
        perror("waitpid");
        exit(EXIT_FAILURE);
    }
    return status;
}

/* -x: un comando, y esperar a que acabe */
static void run_one(const char *command)
{
    int cmd_argc;
    char **cmd_argv = parse_command(command, &cmd_argc);

    if (cmd_argc > 0)
        wait_for(launch_command(cmd_argv));
    free_command(cmd_argv);
}

/* -s sin -b: un comando por línea, cada uno espera al anterior */
static void run_file_sequential(FILE *file)
{
    char line[MAX_LINE];
    int number = 0;

    while (fgets(line, sizeof(line), file) != NULL) {
        int cmd_argc;
        line[strcspn(line, "\n")] = '\0';      /* quitar el salto de línea */
        char **cmd_argv = parse_command(line, &cmd_argc);

        if (cmd_argc > 0) {
            printf("@@ Running command #%d: %s\n", number, line);
            fflush(stdout);                    /* que salga antes que la salida del hijo */
            pid_t pid = launch_command(cmd_argv);
            int status = wait_for(pid);
            printf("@@ Command #%d terminated (pid: %d, status: %d)\n", number, pid, status);
            number++;
        }
        free_command(cmd_argv);
    }
}

/* -s -b: lanzar todos sin esperar y luego recoger terminaciones en el orden en que ocurran */
static void run_file_background(FILE *file)
{
    char line[MAX_LINE];
    pid_t pids[MAX_COMMANDS];
    int launched = 0;

    while (launched < MAX_COMMANDS && fgets(line, sizeof(line), file) != NULL) {
        int cmd_argc;
        line[strcspn(line, "\n")] = '\0';
        char **cmd_argv = parse_command(line, &cmd_argc);

        if (cmd_argc > 0) {
            printf("@@ Running command #%d: %s\n", launched, line);
            fflush(stdout);
            pids[launched] = launch_command(cmd_argv);
            launched++;
        }
        free_command(cmd_argv);
    }

    for (int remaining = launched; remaining > 0; remaining--) {
        int status;
        pid_t pid = wait(&status);           /* el primero que termine, sea cual sea */
        if (pid == -1) {
            perror("wait");
            exit(EXIT_FAILURE);
        }
        for (int number = 0; number < launched; number++)    /* PID -> nº de comando */
            if (pids[number] == pid)
                printf("@@ Command #%d terminated (pid: %d, status: %d)\n", number, pid, status);
    }
}

int main(int argc, char *argv[])
{
    const char *command = NULL;
    const char *script = NULL;
    int background = 0;
    int opt;

    while ((opt = getopt(argc, argv, "x:s:b")) != -1) {
        switch (opt) {
        case 'x': command = optarg;  break;
        case 's': script = optarg;   break;
        case 'b': background = 1;    break;
        default:
            fprintf(stderr, "Usage: %s [-x \"command\"] [-s file [-b]]\n", argv[0]);
            return EXIT_FAILURE;
        }
    }
    if (command == NULL && script == NULL) {
        fprintf(stderr, "Usage: %s [-x \"command\"] [-s file [-b]]\n", argv[0]);
        return EXIT_FAILURE;
    }

    if (command != NULL)
        run_one(command);

    if (script != NULL) {
        FILE *file = fopen(script, "r");
        if (file == NULL) {
            perror(script);
            return EXIT_FAILURE;
        }
        if (background)
            run_file_background(file);
        else
            run_file_sequential(file);
        fclose(file);
    }
    return EXIT_SUCCESS;
}
