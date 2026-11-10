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

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;				/* mutex para proteger el acceso a la disco */
static pthread_cond_t  normal_changed = PTHREAD_COND_INITIALIZER; 		/* condición para avisar de cambios en el estado de la disco */
static pthread_cond_t  vip_changed = PTHREAD_COND_INITIALIZER;			/* condición para avisar de cambios en el estado de la disco para vips */

static int inside = 0;          /* clientes dentro */
static int vip_waiting = 0;     /* vips esperando */
static int next_vip_ticket = 0, vip_turn = 0; 		/* tickets y turnos para vips */
static int next_normal_ticket = 0, normal_turn = 0; /* tickets y turnos para normales */

static void wake_next(void)
{
	if (vip_waiting > 0)
		pthread_cond_broadcast(&vip_changed);
	else
		pthread_cond_broadcast(&normal_changed);
}

void enter_normal_client(int id)
{
	/* Los clientes normales esperan a que no haya vips esperando y a que haya espacio en la disco */
	pthread_mutex_lock(&mutex);
	int my_ticket = next_normal_ticket++;
	printf("Client %2d (not vip) waiting at the door\n", id);
	/* Esperamos a que sea nuestro turno y que haya espacio en la disco */
	while (my_ticket != normal_turn || inside >= CAPACITY || vip_waiting > 0)
		pthread_cond_wait(&normal_changed, &mutex);
	/* Una vez que es nuestro turno y hay espacio, entramos */
	normal_turn++;
	inside++;
	printf("Client %2d (not vip) ENTERS  [inside: %d/%d]\n", id, inside, CAPACITY);
	if (inside < CAPACITY) wake_next();
	pthread_mutex_unlock(&mutex);
}

void enter_vip_client(int id)
{
	/* Los clientes vip tienen prioridad sobre los normales */
	pthread_mutex_lock(&mutex);
	int my_ticket = next_vip_ticket++;
	vip_waiting++;
	printf("Client %2d (  vip  ) waiting at the door\n", id);
	/* Esperamos a que sea nuestro turno y que haya espacio en la disco */
	while (my_ticket != vip_turn || inside >= CAPACITY)
		pthread_cond_wait(&vip_changed, &mutex);
	/* Una vez que es nuestro turno y hay espacio, entramos */
	vip_waiting--;
	vip_turn++;
	inside++;
	printf("Client %2d (  vip  ) ENTERS  [inside: %d/%d]\n", id, inside, CAPACITY);
	if (inside < CAPACITY) wake_next();
	pthread_mutex_unlock(&mutex);
}

void dance(int id, int isvip)
{
	/* Bailamos un tiempo aleatorio */
	printf("Client %2d (%s) dancing in disco\n", id, VIPSTR(isvip));
	sleep((rand() % 3) + 1);
}

void disco_exit(int id, int isvip)
{
	/* Salimos de la disco*/
	pthread_mutex_lock(&mutex);
	inside--;
	printf("Client %2d (%s) LEAVES   [inside: %d/%d]\n", id, VIPSTR(isvip), inside, CAPACITY);
	/* Avisamos a los demás hilos de que ha cambiado el estado, no ponemos condicion porque sabemos que hay espacio */
	wake_next();
	/* Avisamos a los demás hilos de que ha cambiado el estado */
	pthread_mutex_unlock(&mutex);
}

void *client(void *arg)
{
	/* Copiamos los argumentos y liberamos la memoria */
	struct client_args args = *(struct client_args *)arg;
	free(arg);

	/* Procesamos el cliente */
	if (args.isvip)
		enter_vip_client(args.id);
	else
		enter_normal_client(args.id);
	/* Bailamos un tiempo aleatorio */
	dance(args.id, args.isvip);
	disco_exit(args.id, args.isvip);
	return NULL;
}

int main(int argc, char *argv[])
{
	/* Comprobación de argumentos y apertura del fichero */
	if (argc != 2) {
		fprintf(stderr, "Uso: %s fichero\n", argv[0]);
		return 1;
	}

	/* Abrimos el fichero y leemos el número de clientes */
	FILE *file = fopen(argv[1], "r");
	if (file == NULL) {
		perror("fopen");
		return 1;
	}

	/* Leemos el número de clientes */
	int num_clients;
	if (fscanf(file, "%d", &num_clients) != 1 || num_clients <= 0) {
		fprintf(stderr, "Fichero con formato incorrecto\n");
		fclose(file);
		return 1;
	}

	/* Leemos los datos de los clientes */
	pthread_t *threads = malloc(num_clients * sizeof(pthread_t));
	for (int id = 0; id < num_clients; id++) {
		/* Creamos un struct para pasar los argumentos al hilo */
		struct client_args *args = malloc(sizeof(*args));
		args->id = id;
		/* Leemos si el cliente es vip o no */
		if (fscanf(file, "%d", &args->isvip) != 1) {
			fprintf(stderr, "Faltan clientes en el fichero\n");
			free(args);
			num_clients = id;
			break;
		}
		/* Creamos el hilo */
		int err = pthread_create(&threads[id], NULL, client, args);
		if (err != 0) {
			fprintf(stderr, "pthread_create: %d\n", err);
			free(args);
			num_clients = id;
			break;
		}
	}
	/* Esperamos a que todos los hilos terminen */
	fclose(file);

	/* Unimos los hilos */
	for (int id = 0; id < num_clients; id++)
		pthread_join(threads[id], NULL);

	/* Liberamos la memoria */
	free(threads);
	pthread_mutex_destroy(&mutex);
	pthread_cond_destroy(&normal_changed);
	pthread_cond_destroy(&vip_changed);
	return 0;
}
