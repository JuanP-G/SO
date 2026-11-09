#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <string.h>

#define NUM_THREADS 5

struct thread_args {
	int number; 	// Thread number
	char priority;	// 'P' for high priority, 'N' for normal priority
};

void *thread_usuario(void *arg){
	struct thread_args args = *(struct thread_args *)arg;
	free(arg); // Free the allocated memory for arguments

	// Get the thread ID
	pthread_t tid = pthread_self();
	printf("Hilo id=%lu, numero=%d, prioridad=%c\n", (unsigned long)tid, args.number, args.priority);
	return NULL;
}

int main(int argc, char* argv[]){
	pthread_t threads[NUM_THREADS];

	(void)argc; // Unused parameter
	(void)argv; // Unused parameter

	for(int i = 0; i < NUM_THREADS; i++){
		struct thread_args *args = malloc(sizeof(*args));

		if (args == NULL) {
			perror("malloc");
			return 1;
		}

		args->number = i;
		args->priority = (i % 2 == 0) ? 'P' : 'N';

		// Create the thread and pass the arguments
		int error = pthread_create(&threads[i], NULL, thread_usuario, args);
		// Check for errors in thread creation
		if (error != 0) {
			fprintf(stderr, "pthread_create: %s\n", strerror(error));
            free(args); // Free the allocated memory in case of error
			return 1;
		}
	}

	// Wait for all threads to finish
	for(int i = 0; i < NUM_THREADS; i++){
		pthread_join(threads[i], NULL);
	}

	return 0;
}
