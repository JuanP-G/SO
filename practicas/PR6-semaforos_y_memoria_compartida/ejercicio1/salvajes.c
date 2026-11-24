#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include "caldero.h"

static int *servings;
static sem_t *mutex, *empty, *full;

int getServingsFromPot(void)
{
	int left;

	sem_wait(mutex);                   /* un salvaje a la vez en el caldero */
	if (*servings == 0) {
		printf("Savage %d: pot empty, calling the cook\n", getpid());
		sem_post(empty);           /* avisa al cocinero */
		sem_wait(full);            /* espera la reposicion (con mutex: nadie mas se cuela) */
	}
	(*servings)--;
	left = *servings;
	printf("Savage %d: took a serving (%d left)\n", getpid(), left);
	sem_post(mutex);
	return left;
}

void eat(void)
{
	unsigned long id = (unsigned long) getpid();
	printf("Savage %lu eating\n", id);
	sleep(rand() % 5);
}

void savages(void)
{
	for (int i = 0; i < NUMITER; i++) {
		getServingsFromPot();
		eat();
	}
}

int main(int argc, char *argv[])
{
	int fd = shm_open(SHM_NAME, O_RDWR, 0);
	if (fd == -1) {
		fprintf(stderr, "No hay caldero: ejecuta primero ./cocinero\n");
		return 1;
	}
	servings = mmap(NULL, sizeof(*servings), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	close(fd);
	if (servings == MAP_FAILED) {
		perror("mmap");
		return 1;
	}

	mutex = sem_open(SEM_MUTEX, 0);
	empty = sem_open(SEM_EMPTY, 0);
	full  = sem_open(SEM_FULL, 0);
	if (mutex == SEM_FAILED || empty == SEM_FAILED || full == SEM_FAILED) {
		fprintf(stderr, "No hay caldero: ejecuta primero ./cocinero\n");
		return 1;
	}

	srand(getpid());                   /* cada proceso con su semilla */
	savages();

	sem_close(mutex);
	sem_close(empty);
	sem_close(full);
	munmap(servings, sizeof(*servings));
	return 0;
}
