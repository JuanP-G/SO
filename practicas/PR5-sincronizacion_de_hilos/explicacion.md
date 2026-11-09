[Volver al inicio](../../README.md) · Anterior: [P4 · Corrección comentada](../PR4-sincronizacion-hilos/correccion-comentada.md)

# P5 · Discoteca con aforo y VIP

Un solo ejercicio: **M hilos cliente**, aforo **N = 5**, clientes `vip` y normales.

## 1. Qué nos dan (`disco.c`)

Solo el esqueleto: `CAPACITY 5`, `VIPSTR`, `dance()` (printf + sleep) y funciones vacías `enter_normal_client`, `enter_vip_client`, `disco_exit`, `client`, `main`. Hay que rellenar todo lo demás.

## 2. Reglas -> condiciones

| Regla | Cómo se garantiza |
|---|---|
| Aforo lleno -> esperar | `inside >= CAPACITY` bloquea |
| VIP antes que normales | el normal también espera si `vip_waiting > 0` |
| Orden de llegada dentro de cada grupo | **turnos** (ticket) por grupo |

## 3. Herramientas

| Herramienta | Para qué |
|---|---|
| `pthread_mutex_t` | exclusión mutua: solo un hilo toca los contadores a la vez |
| `pthread_cond_wait(&cond, &mutex)` | suelta el mutex y duerme; al despertar lo recupera |
| `pthread_cond_broadcast(&cond)` | despierta a TODOS los dormidos |
| `while` (no `if`) | al despertar hay que **volver a comprobar** la condición |

Una sola variable de condición (`changed`) + `broadcast` en cada cambio de estado: simple y correcto (cada hilo re-comprueba su propia condición).

## 4. Estado compartido (todo protegido por `mutex`)

| Variable | Significado |
|---|---|
| `inside` | clientes dentro |
| `vip_waiting` | vips que han llegado y aún no han entrado |
| `next_vip_ticket` / `vip_turn` | dispensador / cartel de turno VIP |
| `next_normal_ticket` / `normal_turn` | ídem para normales |

## 5. Cómo funciona cada función

**`enter_vip_client`**: coge el mutex, saca ticket, `vip_waiting++`, espera mientras `ticket != vip_turn || inside >= CAPACITY`. Al entrar: `vip_waiting--`, `vip_turn++`, `inside++`, `broadcast`.

**`enter_normal_client`**: igual, pero espera mientras `ticket != normal_turn || inside >= CAPACITY || vip_waiting > 0`.

**`dance`**: **sin** el mutex cogido (si no, nadie podría entrar ni salir).

**`disco_exit`**: mutex, `inside--`, `broadcast`.

**Por qué `broadcast` también al entrar**: al entrar un VIP, `vip_waiting` baja o `vip_turn` sube, y eso puede desbloquear al siguiente VIP o a los normales.

**`client`**: copia los argumentos (`struct client_args`) a la pila, hace `free(arg)`, y llama a `enter_*` -> `dance` -> `disco_exit`.

**`main`**: `fopen` + `fscanf` (M y luego un 0/1 por cliente), un `malloc` por hilo para sus argumentos (así no hay carrera con una variable compartida), `pthread_create` (comprueba el error), `pthread_join` de todos, y al final `destroy` de mutex y cond.

## 6. Código de solución

```c
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
```

## 7. Prueba realizada

Con `grande.txt` (14 clientes, 5 vips): el máximo `inside` impreso fue **5/5** y el orden de entrada fue

`1 0 2 3 4` (normales que llegaron primero) -> `8 6 9 11` (VIPs, se saltan a los normales que esperaban) -> `5 7 13 12 10` (normales restantes).

Compilar y ejecutar: `make && ./disco ejemplo.txt` .

## 8. Errores típicos

| Error | Consecuencia |
|---|---|
| `if` en vez de `while` alrededor de `cond_wait` | entra alguien sin cumplir la condición |
| Bailar con el mutex cogido | todos bloqueados, nadie entra/sale |
| Olvidar `broadcast` al salir (o al entrar) | clientes dormidos para siempre |
| Un único `int` compartido para los argumentos del hilo | carrera: dos hilos con el mismo `id` |
| Normal sin comprobar `vip_waiting` | los VIP no tienen prioridad |
| Sin turnos | no se respeta el orden de llegada |
