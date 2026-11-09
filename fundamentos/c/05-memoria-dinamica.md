# SO - C 05 - Memoria dinamica

Parte de [Inicio](../../README.md). Anterior: [C 04 - Punteros](04-punteros.md) | Siguiente: [C 06 - Funciones y structs](06-funciones-y-structs.md)

---

## 1. El mapa de memoria de un proceso

Esto es teoria del **Modulo 5** y a la vez practica de todos los dias.

```
  direcciones altas
 ┌──────────────────────┐
 │   argumentos y env   │
 ├──────────────────────┤
 │        PILA          │  variables locales, parametros, direcciones de retorno
 │      (stack)         │  crece HACIA ABAJO
 │          │           │  se gestiona SOLA (entrar/salir de funcion)
 │          v           │  tamano limitado (~8 MB)
 │                      │
 │          ^           │
 │          │           │
 │      MONTICULO       │  malloc / free
 │       (heap)         │  crece HACIA ARRIBA
 ├──────────────────────┤  lo gestionas TU
 │   .bss               │  globales y static SIN inicializar -> puestas a 0
 ├──────────────────────┤
 │   .data              │  globales y static CON valor inicial
 ├──────────────────────┤
 │   .rodata            │  constantes y literales de cadena (SOLO LECTURA)
 ├──────────────────────┤
 │   .text              │  el codigo maquina del programa (solo lectura)
 └──────────────────────┘
  direcciones bajas
```

| Donde | Quien lo gestiona | Cuando muere | Tamano |
|-------|-------------------|--------------|--------|
| **Pila** | el compilador | al salir de la funcion | limitado |
| **Heap** | **tu**, con `malloc`/`free` | cuando haces `free` | grande |
| **.data / .bss** | el compilador | fin del programa | fijo |

> [!tip] Comprueba el mapa tu mismo
> En `main2.c` de la practica, imprime las direcciones de una global, una local y un `malloc`. Veras que caen en zonas muy distintas. Eso es este dibujo, en vivo.

---

## 2. `malloc` y `free`

```c
#include <stdlib.h>

int *v = malloc(100 * sizeof(int));   /* reserva 400 bytes */
if (v == NULL) {                      /* comprobar SIEMPRE */
    perror("malloc");
    exit(1);
}

v[0] = 42;
/* ... usar v ... */

free(v);        /* devolver la memoria */
v = NULL;       /* buena costumbre */
```

| Funcion | Que hace |
|---------|----------|
| `malloc(n)` | reserva n **bytes**, sin inicializar (basura) |
| `calloc(n, tam)` | reserva n elementos de tam bytes, **todo a 0** |
| `realloc(p, n)` | cambia el tamano de un bloque, copiando el contenido |
| `free(p)` | devuelve el bloque al sistema |

> [!important] `malloc` cuenta BYTES, no elementos
> ```c
> int *v = malloc(100);              /* MAL: 100 bytes = solo 25 ints */
> int *v = malloc(100 * sizeof(int)); /* BIEN */
> ```
> Usa siempre `sizeof`. No supongas que `int` mide 4.

Para cadenas, acuerdate del `'\0'`:

```c
char *copia = malloc(strlen(orig) + 1);   /* el +1 es OBLIGATORIO */
strcpy(copia, orig);
```

---

## 3. Cuando hace falta memoria dinamica

```c
int v[n];        /* solo funciona si n se conoce... y aun asi es mala idea */
```

| Situacion | Solucion |
|-----------|----------|
| El tamano se sabe al compilar | array normal, en la pila |
| El tamano depende de la ejecucion (argumentos, fichero...) | `malloc` |
| El dato tiene que **sobrevivir a la funcion** que lo crea | `malloc` |
| Es muy grande (megas) | `malloc` (la pila es pequena) |

> [!danger] El error mas grave: devolver un puntero a una variable local
> ```c
> char *saludo(void) {
>     char buf[100];
>     strcpy(buf, "hola");
>     return buf;         /* buf MUERE al salir de la funcion */
> }                       /* devuelves un puntero a memoria destruida */
> ```
> Correcto:
> ```c
> char *saludo(void) {
>     char *buf = malloc(100);
>     strcpy(buf, "hola");
>     return buf;         /* el heap sobrevive. El que llame debe hacer free */
> }
> ```
> Esta es la razon de ser de `clone_string()` / `strdup()` en el ejercicio 4 de la Practica 1: cada campo de la estructura necesita su propia copia que sobreviva a la iteracion del bucle.

---

## 4. Los cuatro errores clasicos de memoria

| Error | Que es | Sintoma |
|-------|--------|---------|
| **Memory leak** | reservas y nunca liberas | el programa come RAM sin parar |
| **Use after free** | usas el puntero despues de `free` | corrupcion silenciosa, luego crash |
| **Double free** | dos `free` sobre el mismo puntero | abort inmediato |
| **Buffer overflow** | escribes fuera del bloque | machacas datos vecinos |

```c
/* LEAK */
p = malloc(100);
p = malloc(200);       /* perdiste la direccion del primero: 100 bytes irrecuperables */

/* USE AFTER FREE */
free(p);
p[0] = 1;              /* punteros3.c y punteros2.c de la practica */

/* DOUBLE FREE */
free(p);
free(p);

/* OVERFLOW */
int *v = malloc(5 * sizeof(int));
v[5] = 1;              /* el ultimo valido es v[4] */
```

### Detectarlos: `valgrind`

```bash
sudo apt install valgrind
gcc -g -Wall prog.c -o prog
valgrind --leak-check=full ./prog
```

Te dice exactamente en que linea reservaste memoria que no liberaste y donde leiste fuera de rango. **Es la herramienta que mas puntos salva en las practicas de SO.**

---

## 5. Regla de oro

> **Cada `malloc` tiene su `free`.**
> Si lo escribes a la vez que el `malloc`, no se te olvida.

```c
char *buf = malloc(n);
if (!buf) { perror("malloc"); return -1; }
/* ... */
free(buf);
```

Y si una funcion devuelve memoria reservada, **documenta quien tiene que liberarla**.

---

## Ejercicios

1. Reserva un array de `n` enteros donde `n` venga del primer argumento, rellenalo y liberalo.
2. Quita el `free` y pasalo por `valgrind`. Lee el informe.
3. Haz `free` dos veces y mira que pasa.
4. Escribe una funcion que reciba una cadena y devuelva una copia reservada con `malloc`.
5. Compara las direcciones de: una global, una local y un `malloc`. Situalas en el mapa de memoria.
6. Reserva 5 enteros y escribe en el indice 5. Pasa `valgrind`. Que dice?
