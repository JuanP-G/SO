#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#define CAPACITY 5
#define VIPSTR(vip) ((vip) ? "  vip  " : "not vip")

struct client_args {
	int id;
	int isvip;
};

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t  changed = PTHREAD_COND_INITIALIZER;

static int inside = 0;          /* clientes dentro */
static int vip_waiting = 0;     /* vips esperando */
static int next_vip_ticket = 0, vip_turn = 0;
static int next_normal_ticket = 0, normal_turn = 0;

void enter_normal_client(int id)
{
	pthread_mutex_lock(&mutex);
	int my_ticket = next_normal_ticket++;
	printf("Client %2d (not vip) waiting at the door\n", id);
	while (my_ticket != normal_turn || inside >= CAPACITY || vip_waiting > 0)
		pthread_cond_wait(&changed, &mutex);
	normal_turn++;
	inside++;
	printf("Client %2d (not vip) ENTERS  [inside: %d/%d]\n", id, inside, CAPACITY);
	pthread_cond_broadcast(&changed);
	pthread_mutex_unlock(&mutex);
}

void enter_vip_client(int id)
{
	pthread_mutex_lock(&mutex);
	int my_ticket = next_vip_ticket++;
	vip_waiting++;
	printf("Client %2d (  vip  ) waiting at the door\n", id);
	while (my_ticket != vip_turn || inside >= CAPACITY)
		pthread_cond_wait(&changed, &mutex);
	vip_waiting--;
	vip_turn++;
	inside++;
	printf("Client %2d (  vip  ) ENTERS  [inside: %d/%d]\n", id, inside, CAPACITY);
	pthread_cond_broadcast(&changed);
	pthread_mutex_unlock(&mutex);
}

void dance(int id, int isvip)
{
	printf("Client %2d (%s) dancing in disco\n", id, VIPSTR(isvip));
	sleep((rand() % 3) + 1);
}

void disco_exit(int id, int isvip)
{
	pthread_mutex_lock(&mutex);
	inside--;
	printf("Client %2d (%s) LEAVES   [inside: %d/%d]\n", id, VIPSTR(isvip), inside, CAPACITY);
	pthread_cond_broadcast(&changed);
	pthread_mutex_unlock(&mutex);
}

void *client(void *arg)
{
	struct client_args args = *(struct client_args *)arg;
	free(arg);

	if (args.isvip)
		enter_vip_client(args.id);
	else
		enter_normal_client(args.id);
	dance(args.id, args.isvip);
	disco_exit(args.id, args.isvip);
	return NULL;
}

int main(int argc, char *argv[])
{
	if (argc != 2) {
		fprintf(stderr, "Uso: %s fichero\n", argv[0]);
		return 1;
	}
	FILE *file = fopen(argv[1], "r");
	if (file == NULL) {
		perror("fopen");
		return 1;
	}
	int num_clients;
	if (fscanf(file, "%d", &num_clients) != 1 || num_clients <= 0) {
		fprintf(stderr, "Fichero con formato incorrecto\n");
		fclose(file);
		return 1;
	}

	pthread_t *threads = malloc(num_clients * sizeof(pthread_t));
	for (int id = 0; id < num_clients; id++) {
		struct client_args *args = malloc(sizeof(*args));
		args->id = id;
		if (fscanf(file, "%d", &args->isvip) != 1) {
			fprintf(stderr, "Faltan clientes en el fichero\n");
			free(args);
			num_clients = id;
			break;
		}
		int err = pthread_create(&threads[id], NULL, client, args);
		if (err != 0) {
			fprintf(stderr, "pthread_create: %d\n", err);
			free(args);
			num_clients = id;
			break;
		}
	}
	fclose(file);

	for (int id = 0; id < num_clients; id++)
		pthread_join(threads[id], NULL);

	free(threads);
	pthread_mutex_destroy(&mutex);
	pthread_cond_destroy(&changed);
	return 0;
}
