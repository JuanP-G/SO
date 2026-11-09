/* Experimento del enunciado: UNA sola variable de argumentos para todos los hilos.
 * Compilar con -DRETRASO=1 para que el hilo tarde un poco en leerla (hace visible la carrera). */
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

#define NUM_THREADS 5

struct thread_args {
    int  number;
    char priority;
};

void *thread_usuario(void *arg)
{
#ifdef RETRASO
    usleep(100000);                       /* 100 ms: el main ya habrá cambiado la variable */
#endif
    struct thread_args args = *(struct thread_args *)arg;     /* se lee DESPUÉS de crear el hilo */
    printf("Hilo numero=%d, prioridad=%c\n", args.number, args.priority);
    return NULL;
}

int main(void)
{
    pthread_t threads[NUM_THREADS];
    struct thread_args shared;            /* UNA variable, compartida por todos */

    for (int i = 0; i < NUM_THREADS; i++) {
        shared.number = i;
        shared.priority = (i % 2 == 0) ? 'P' : 'N';
        pthread_create(&threads[i], NULL, thread_usuario, &shared);
    }
    for (int i = 0; i < NUM_THREADS; i++)
        pthread_join(threads[i], NULL);
    return 0;
}
