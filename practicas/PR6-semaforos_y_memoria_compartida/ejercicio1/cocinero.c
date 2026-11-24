#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include "caldero.h"

static volatile sig_atomic_t finish = 0;

static int *servings;                  /* raciones: vive en memoria compartida */
static sem_t *mutex, *empty, *full;

void putServingsInPot(int n)
{
	if (sem_wait(empty) == -1)         /* duerme hasta que un salvaje avise */
		return;                    /* EINTR: senal, salimos a mirar finish */
	*servings = n;
	printf("Cook %d: pot refilled with %d servings\n", getpid(), n);
	sem_post(full);                    /* avisa al salvaje que espera */
}

void cook(void)
{
	while (!finish) {
		putServingsInPot(M);
	}
}

void handler(int signo)
{
	finish = 1;
}

static void cleanup(void)
{
	munmap(servings, sizeof(*servings));
	shm_unlink(SHM_NAME);
	sem_close(mutex);  sem_unlink(SEM_MUTEX);
	sem_close(empty);  sem_unlink(SEM_EMPTY);
	sem_close(full);   sem_unlink(SEM_FULL);
}

int main(int argc, char *argv[])
{
	/* borra restos de una ejecucion anterior que no limpio */
	shm_unlink(SHM_NAME);
	sem_unlink(SEM_MUTEX);
	sem_unlink(SEM_EMPTY);
	sem_unlink(SEM_FULL);

	int fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0600);
	if (fd == -1) {
		perror("shm_open");
		return 1;
	}
	if (ftruncate(fd, sizeof(*servings)) == -1) {
		perror("ftruncate");
		return 1;
	}
	servings = mmap(NULL, sizeof(*servings), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	close(fd);                         /* tras mmap el fd ya no hace falta */
	if (servings == MAP_FAILED) {
		perror("mmap");
		return 1;
	}
	*servings = 0;                     /* el caldero empieza vacio */

	mutex = sem_open(SEM_MUTEX, O_CREAT, 0600, 1);
	empty = sem_open(SEM_EMPTY, O_CREAT, 0600, 0);
	full  = sem_open(SEM_FULL,  O_CREAT, 0600, 0);
	if (mutex == SEM_FAILED || empty == SEM_FAILED || full == SEM_FAILED) {
		perror("sem_open");
		return 1;
	}

	/* sin SA_RESTART para que la senal interrumpa sem_wait (EINTR) */
	struct sigaction sa = { .sa_handler = handler };
	sigemptyset(&sa.sa_mask);
	sigaction(SIGTERM, &sa, NULL);
	sigaction(SIGINT, &sa, NULL);

	printf("Cook %d: ready, waiting for savages\n", getpid());
	cook();

	printf("\nCook %d: cleaning up\n", getpid());
	cleanup();
	return 0;
}
