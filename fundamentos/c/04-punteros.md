# SO - C 04 - Punteros

Parte de [Inicio](../../README.md). Anterior: [C 03 - Arrays y strings](03-arrays-y-strings.md) | Siguiente: [C 05 - Memoria dinamica](05-memoria-dinamica.md)

> [!important] Esta es LA nota
> Si entiendes punteros, C deja de dar miedo. Si no, todo lo demas de la asignatura (procesos, ficheros, memoria) va a ser magia negra. Merece la pena pelearse con esto hasta que haga clic.

---

## 1. La idea

La memoria es un array gigante de bytes. Cada byte tiene un **numero** que lo identifica: su **direccion**.

Un **puntero es una variable cuyo valor es una direccion de memoria.**

```
  int c = 7;
  int *ptr = &c;

      ptr                          c
  ┌───────────┐              ┌───────────┐
  │  0x1000   │ ───────────> │     7     │
  └───────────┘              └───────────┘
   dir: 0x2000                dir: 0x1000

  ptr   vale 0x1000     (la direccion de c)
  *ptr  vale 7          (lo que hay EN esa direccion)
  &ptr  vale 0x2000     (donde vive el propio ptr)
```

---

## 2. Los dos operadores

| Operador | Nombre | Lee como | Ejemplo |
|----------|--------|----------|---------|
| `&x` | **direccion de** | "donde vive x" | `&c` -> 0x1000 |
| `*p` | **indireccion / contenido de** | "lo que hay en p" | `*ptr` -> 7 |

Son inversos: `*(&c)` es `c`.

> [!warning] El `*` significa dos cosas distintas
> ```c
> int *ptr;      /* DECLARACION: "ptr es un puntero a int".  Aqui * es parte del tipo */
> *ptr = 4;      /* USO: "escribe 4 donde apunta ptr".       Aqui * es indireccion */
> ```
> Es la confusion numero 1 de todo el mundo. En la declaracion es tipo, en el codigo es operacion.

```c
int  *p;       /* las tres formas son identicas */
int*  p;
int * p;

int* a, b;     /* TRAMPA: a es int*, pero b es int normal */
int *a, *b;    /* asi si son los dos punteros */
```

---

## 3. Recorrido comentado de `punteros1.c` (ejercicio 1.5)

```c
int c = 7;              /* global */
int *ptr;               /* local SIN inicializar -> apunta a BASURA */

printf(..., ptr, ...);  /* imprime una direccion aleatoria: PELIGRO */

ptr = &c;               /* ahora ptr apunta a c */
*ptr = 4;               /* escribe 4 en c.  Ahora c vale 4, sin tocar 'c' */

ptr = (int*) 0x600a48;  /* apuntar a una direccion INVENTADA */
*ptr = 13;              /* escribir ahi  ->  SEGMENTATION FAULT */
```

**Respuestas del enunciado:**

- *Operador para declarar un puntero*: `*`
- *Operador para obtener la direccion de una variable*: `&`
- *Operador para acceder al contenido apuntado*: `*`
- *Hay un error. Se produce en compilacion o en ejecucion?* **En ejecucion.** Compila perfectamente. `ptr = (int*)0x600a48` asigna una direccion arbitraria que no pertenece al proceso; al escribir ahi con `*ptr = 13` el SO detecta el acceso invalido y mata el proceso con SIGSEGV. El compilador no puede saber en tiempo de compilacion que esa direccion es invalida.

> [!note] Que es un "segmentation fault"
> El SO le da a cada proceso un espacio de direcciones propio, dividido en regiones con permisos. Si intentas leer o escribir fuera de las regiones que tienes asignadas (o escribir en una de solo lectura), la MMU lanza una excepcion, el kernel te manda la senal **SIGSEGV** y el proceso muere. Es el SO protegiendote a ti y al resto del sistema. Modulo 5.

---

## 4. Punteros y arrays: casi lo mismo, pero no

**El nombre de un array es la direccion de su primer elemento.**

```c
int v[5];
v  ==  &v[0]      /* cierto */
```

```c
int v[5] = {10,20,30,40,50};
int *p = v;         /* sin &, el array ya es una direccion */

v[2]        /* 30 */
p[2]        /* 30  -> los punteros tambien se indexan */
*(v+2)      /* 30  -> notacion equivalente */
*(p+2)      /* 30 */
```

**`p[i]` es exactamente `*(p + i)`.** Son la misma cosa escrita de dos maneras.

### Aritmetica de punteros

```c
int *p;         /* int ocupa 4 bytes */
p     -> 0x1000
p + 1 -> 0x1004      /* NO 0x1001!  suma 1 * sizeof(int) */
p + 3 -> 0x100C

char *q;        /* char ocupa 1 byte */
q + 1 -> q + 1 byte
```

> [!important] Sumar 1 a un puntero avanza UN ELEMENTO, no un byte
> El compilador multiplica por `sizeof(tipo)` automaticamente. Por eso `ptr2++` en `punteros2.c` pasa de `ptr[0]` a `ptr[1]`.

### Diferencias reales entre array y puntero

| | `int v[5]` | `int *p` |
|---|-----------|----------|
| `sizeof` | 20 (todo el array) | 8 (solo el puntero) |
| Se puede reasignar? | **No** (`v = otra` es error) | Si (`p = otra`) |
| Reserva memoria para los datos? | Si, 20 bytes | No, solo 8 para la direccion |

---

## 5. Recorrido de `punteros2.c`

```c
ptr = (int*) malloc(nelem * sizeof(int));   /* nelem = 127 */
```

**"Cuantos bytes se reservan?"** -> `127 * sizeof(int)` = `127 * 4` = **508 bytes**.
Primer byte: la direccion que devuelve `malloc`. Ultimo byte: esa direccion + 507.

```c
*ptr = 5;       /* escribe 5 en ptr[0] */
ptr[0] = 7;     /* SOBRESCRIBE con 7.  *ptr y ptr[0] son lo mismo */
printf(... *ptr ...);   /* imprime 7, no 5.  Esa es la pregunta del enunciado */

ptr2 = ptr;     /* ptr2 apunta al MISMO sitio, no hay copia */
ptr[1] = 10;
ptr2++;         /* ptr2 avanza 4 bytes -> ahora apunta a ptr[1] */
*ptr2 = 15;     /* por tanto ptr[1] pasa a valer 15 */
```

**"Tres formas distintas de escribir 13 en `ptr[100]`":**

```c
ptr[100] = 13;
*(ptr + 100) = 13;
int *q = ptr + 100;  *q = 13;
/* tambien:  *(100 + ptr) = 13;   e incluso  100[ptr] = 13;  (legal, horrible) */
```

**"Hay un error aunque no se manifieste. Cual es?"**

```c
free(ptr);      /* se devuelve la memoria al sistema */
*ptr = 3;       /* USE AFTER FREE: escribir en memoria ya liberada */
```

Esto es un **use-after-free**. Muchas veces "funciona" porque el sistema todavia no ha reutilizado esa zona, pero es un error grave: en cuanto otra parte del programa pida memoria, estaras corrompiendo datos ajenos. Ademas `ptr2` quedo apuntando dentro de la misma zona liberada (*dangling pointer*).

---

## 6. Recorrido de `punteros3.c`

```c
ptr = malloc(127 * sizeof(int));
for (i=0; i<127; i++) ptr[i] = i;
printf("ptr[0]=%d ptr[13]=%d", ...);    /* 0 y 13 */

ptr = &c;       /* ptr deja de apuntar al bloque y apunta a la variable c */
printf("ptr[0]=%d ptr[13]=%d", ...);
```

- `ptr[0]` ahora es `c` = **37**.
- `ptr[13]` es `*(&c + 13)`: lee 52 bytes mas alla de `c`. Ahi **no hay nada tuyo**: imprime basura. En ejecucion, no en compilacion.
- **Que pasa con la zona de `malloc`?** Se ha perdido: ya no queda ningun puntero apuntando a ella. Es una **fuga de memoria (memory leak)**. Es **imposible** acceder a ella ni liberarla, porque perdiste la unica direccion que tenias.
- `free(ptr)` al final es peor todavia: intenta liberar `&c`, que es una variable global, **no una zona de `malloc`**. Comportamiento indefinido / abort.

**La leccion**: antes de reasignar un puntero que apunta a memoria dinamica, o la liberas o guardas la direccion en otra variable.

---

## 7. `NULL` y punteros seguros

```c
int *p = NULL;      /* "no apunto a nada". NULL es la direccion 0 */

if (p != NULL) {
    *p = 5;
}

*p = 5;   /* si p es NULL -> segmentation fault inmediato */
```

> [!tip] Buenas costumbres que te ahorran horas de gdb
> 1. Inicializa siempre los punteros: `int *p = NULL;`
> 2. Comprueba lo que devuelve `malloc` (devuelve `NULL` si no hay memoria).
> 3. Despues de `free(p)`, pon `p = NULL;`. Asi un use-after-free peta al instante en vez de corromper cosas en silencio.
> 4. Compila con `-Wall -g` y depura con `gdb`.

---

## 8. Punteros a punteros

```c
int   c   = 7;
int  *p   = &c;
int **pp  = &p;

*pp    ->  p      (un int*)
**pp   ->  7      (el int)
```

```
    pp              p                c
 ┌────────┐     ┌────────┐      ┌────────┐
 │ &p     │ ──> │ &c     │ ───> │   7    │
 └────────┘     └────────┘      └────────┘
```

Para que sirve: **para que una funcion pueda modificar tu puntero**, no solo lo que apunta. Es exactamente por lo que `strsep()` recibe `char **stringp` y por lo que `main` recibe `char **argv`.

---

## 9. `argc` y `argv`

```c
int main(int argc, char **argv)   /* o: char *argv[] */
```

Ejecutando `./getopt -l 3 hola`:

| | |
|---|---|
| `argc` | **4** (el nombre del programa cuenta) |
| `argv[0]` | `"./getopt"` |
| `argv[1]` | `"-l"` |
| `argv[2]` | `"3"` |
| `argv[3]` | `"hola"` |
| `argv[4]` | `NULL` |

```
  argv
 ┌──────┐     ┌──────┬──────┬──────┬──────┬──────┐
 │  ●───┼───> │  ●   │  ●   │  ●   │  ●   │ NULL │
 └──────┘     └──┬───┴──┬───┴──┬───┴──┬───┘
                 v      v      v      v
            "./getopt" "-l"   "3"   "hola"
```

> [!danger] Los bugs del ejercicio 2 (`primes.c`)
> ```c
> int n = 10;
> if (argc = 2) {          /* TRES errores en dos lineas */
>     atoi(argv[2]);
> }
> ```
> 1. **`argc = 2` es una ASIGNACION, no una comparacion.** Machaca `argc` con 2 y la condicion vale 2, que es distinto de 0, asi que **el `if` se cumple siempre**, ejecutes el programa con argumentos o sin ellos.
> 2. Si de verdad hubiera 2 argumentos, los indices validos serian 0 y 1. **`argv[2]` es `NULL`**, y `atoi(NULL)` peta.
> 3. El resultado de `atoi` **no se asigna a nada**: `n` se queda en 10 pase lo que pase.
>
> Deberia ser:
> ```c
> if (argc == 2) {
>     n = atoi(argv[1]);
> }
> ```
> Y `gcc -Wall` canta los dos primeros solito:
> ```
> warning: suggest parentheses around assignment used as truth value
> warning: statement with no effect
> ```

Los argumentos llegan **siempre como cadenas**. Para convertirlos:

| Funcion | Convierte a | Detecta errores? |
|---------|-------------|------------------|
| `atoi(s)` | `int` | no (devuelve 0 y no distingue de "0") |
| `strtol(s, &fin, 10)` | `long` | **si** |

El enunciado del ejercicio 3 te pide usar `strtol()` precisamente por eso:

```c
long len = strtol(optarg, NULL, 10);   /* 10 = base decimal */
```

---

## Ejercicios

1. Declara un `int`, un puntero a el, e imprime `x`, `&x`, `p`, `*p` y `&p`. Dibuja el esquema.
2. Cambia el valor de `x` a traves del puntero y compruebalo.
3. Recorre un array de 5 enteros de dos formas: con `v[i]` y con `*(v+i)`.
4. Imprime las direcciones de `v[0]`, `v[1]`, `v[2]`. Cuanto se diferencian? Y si fuese `char`?
5. Escribe una funcion `void intercambia(int *a, int *b)` que intercambie dos enteros.
6. Escribe un programa que imprima `argc` y todos los `argv[i]` en un bucle.
7. Arregla el bug de `primes.c` y comprueba que `./primes 5` funciona.
