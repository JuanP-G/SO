#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <string.h>

#define NUM_THREADS 5

struct thread_args {
    int  number;      /* nº de hilo */
    char priority;    /* 'P' prioritario, 'N' no prioritario */
};

void *thread_usuario(void *arg)
{
    struct thread_args args = *(struct thread_args *)arg;   /* copia local */
    free(arg);                                              /* ya no hace falta */

    pthread_t id = pthread_self();
    printf("Hilo id=%lu, numero=%d, prioridad=%c\n",
           (unsigned long)id, args.number, args.priority);
    return NULL;
}

int main(int argc, char *argv[])
{
    pthread_t threads[NUM_THREADS];

    (void)argc;
    (void)argv;

    for (int i = 0; i < NUM_THREADS; i++) {
        struct thread_args *args = malloc(sizeof(*args));   /* uno por hilo */
        if (args == NULL) {
            perror("malloc");
            return 1;
        }
        args->number = i;
        args->priority = (i % 2 == 0) ? 'P' : 'N';

        int error = pthread_create(&threads[i], NULL, thread_usuario, args);
        if (error != 0) {      /* pthread_create devuelve el error, no usa errno */
            fprintf(stderr, "pthread_create: %s\n", strerror(error));
            free(args);
            return 1;
        }
    }

    for (int i = 0; i < NUM_THREADS; i++)
        pthread_join(threads[i], NULL);

    return 0;
}
