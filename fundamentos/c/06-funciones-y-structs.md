# SO - C 06 - Funciones y structs

Parte de [Inicio](../../README.md). Anterior: [C 05 - Memoria dinamica](05-memoria-dinamica.md) | Siguiente: [Herramientas - gcc make gdb](../herramientas-gcc-make-gdb.md)

---

## 1. En C **todo** se pasa por valor

```c
void f(int x) { x = 99; }

int a = 1;
f(a);
printf("%d", a);    /* 1.  NO ha cambiado */
```

La funcion recibe una **copia**. Modificarla no toca el original.

### Para modificar el original: pasar la direccion

```c
void f(int *x) { *x = 99; }

int a = 1;
f(&a);              /* le paso DONDE esta a */
printf("%d", a);    /* 99 */
```

Esto **no es "paso por referencia"** como en Java o C++: sigue siendo paso por valor, lo que pasa es que **el valor que copiamos es una direccion**. El resultado practico es el mismo.

```
f(a):                     f(&a):
  a           x             a          x
┌───┐      ┌───┐        ┌───┐      ┌───────┐
│ 1 │      │ 1 │        │ 1 │ <────│  &a   │
└───┘      └───┘        └───┘      └───────┘
 copia independiente      x apunta al original
```

> [!important] Ejercicio 1.6 (`arg1.c`): por que `xc` no se modifica tras llamar a `sumC`?
> Porque se pasa por valor: dentro de `sumC` se modifica **la copia local**. El cambio existe, pero solo dentro del marco de pila de `sumC`, y desaparece al volver. Para que se viera fuera habria que pasar `&xc` y recibirlo como puntero.

### Excepcion aparente: los arrays

```c
void f(int v[]) { v[0] = 99; }     /* SI modifica el original */
```

Porque un array al pasarlo **se degrada a puntero**: en realidad estas pasando la direccion del primer elemento. No hay copia del array.

---

## 2. Estructuras

```c
struct punto {
    int x;
    int y;
};

struct punto p;
p.x = 3;
p.y = 4;
```

Con `typedef` (como en el `defs.h` de la practica) te ahorras escribir `struct`:

```c
/* el de verdad, en ejercicio4/defs.h de la Practica 1 */
typedef struct passwd_entry {
    char  login_name[MAX_LOGIN_NAME];   /* ARRAY: tiene memoria propia */
    char* optional_encrypted_passwd;    /* PUNTERO: no apunta a nada todavia */
    int   uid;
    int   gid;
    char* user_name;
    char* user_home;
    char* user_shell;
} passwd_entry_t;

passwd_entry_t e;
e.uid = 1000;
```

Fijate en la mezcla: `login_name` es un **array** (64 bytes dentro de la propia estructura), los demas campos de texto son **punteros** (8 bytes que no apuntan a nada hasta que tu los hagas apuntar). Esa diferencia es justo lo que pregunta el enunciado del ejercicio 4.

### `.` vs `->` (pregunta del enunciado)

| Tienes | Usas | Equivale a |
|--------|------|------------|
| la estructura | `e.uid` | |
| un **puntero** a la estructura | `pe->uid` | `(*pe).uid` |

```c
passwd_entry_t  e;
passwd_entry_t *pe = &e;

e.uid = 1000;        /* variable  -> punto */
pe->uid = 1000;      /* puntero   -> flecha */
(*pe).uid = 1000;    /* lo mismo, mas feo */
```

**Regla mnemotecnica**: la flecha `->` "viaja" hasta la estructura porque tienes su direccion, no la estructura.

### Asignacion de estructuras

```c
struct punto a = {1,2};
struct punto b = a;      /* SI se puede: copia campo a campo */
```

Pero ojo: si un campo es un `char*`, **copia el puntero, no el texto**. Los dos apuntarian a la misma cadena. Eso es una *shallow copy*, y es exactamente el problema que resuelve `clone_string()` en el ejercicio 4 de la Practica 1.

---

## 3. Por que `clone_string()` (ejercicio 4 explicado)

En `show-passwd.c`, `parse_passwd()` trocea una linea con `strsep()` y rellena la estructura.

```c
char line[256];
/* se lee una linea de /etc/passwd en line */
char *tok = strsep(&resto, ":");

entry->user_home = tok;                    /* MAL */
strcpy(entry->user_home, tok);             /* MAL: user_home no tiene memoria */
entry->user_home = clone_string(tok);      /* BIEN */

strcpy(entry->login_name, tok);            /* BIEN: login_name SI es un array */
```

| Opcion | Por que falla |
|--------|---------------|
| `entry->user_home = tok;` | `tok` apunta **dentro de `line`**, que es un buffer que se reutiliza en la siguiente iteracion del bucle. Todas las entradas acabarian apuntando a la ultima linea leida. Y si `line` es local, muere al salir de la funcion. |
| `strcpy(entry->user_home, tok);` | `entry->user_home` es un `char*` que **no apunta a ninguna memoria reservada**. `strcpy` escribiria en una direccion basura -> segfault. |
| `clone_string(tok)` | Reserva memoria propia con `malloc` y copia el contenido. La copia sobrevive. |

En cambio, los campos declarados como **array** (`char login_name[MAX_LOGIN_NAME]`) si tienen su propia memoria dentro de la estructura, y ahi `strcpy` si vale.

`clone_string()` se puede sustituir por `strdup()`, que hace exactamente eso:

```c
char *clone_string(const char *s) {
    char *c = malloc(strlen(s) + 1);
    if (c) strcpy(c, s);
    return c;
}
/* equivale a: */
char *c = strdup(s);
```

Y en ambos casos, **hay que hacer `free()`** de cada campo al terminar.

---

## 4. `getopt()` - procesar opciones (ejercicio 3)

```c
#include <unistd.h>

int opt;
while ((opt = getopt(argc, argv, "hel:")) != -1) {
    switch (opt) {
        case 'h': imprimir_ayuda(); exit(0);
        case 'e': par = 1;            break;
        case 'l': len = strtol(optarg, NULL, 10);  break;
        default:  imprimir_ayuda(); exit(1);
    }
}
/* tras el bucle, optind apunta al primer argumento SIN opcion */
titulo = argv[optind];
```

### La cadena `optstring`

| Escribes | Significa |
|----------|-----------|
| `h` | opcion `-h` **sin** argumento |
| `l:` | opcion `-l` **con** argumento obligatorio (`-l 5`) |
| `l::` | argumento opcional (extension GNU) |
| `:` al principio | cambia como se reportan los errores |

Para la practica (`-h`, `-e`, `-l valor`) la respuesta es:

```c
"hel:"
```

### Variables globales que pone `getopt`

| Variable | Contiene |
|----------|----------|
| `char *optarg` | el argumento de la opcion actual (o `NULL`) |
| `int optind` | indice del **siguiente** elemento de `argv` por procesar |
| `int opterr` | poner a 0 para silenciar los mensajes de error automaticos |

### Y la segunda pregunta del enunciado

> "Que linea de codigo utilizas para leer el argumento `title`?"

```c
char *title = argv[optind];
```

Con comprobacion (porque `title` es obligatorio):

```c
if (optind >= argc) {
    fprintf(stderr, "Falta el argumento title\n");
    imprimir_ayuda();
    exit(1);
}
char *title = argv[optind];
```

---

## 5. Devolver varios valores

C solo devuelve una cosa. Trucos habituales:

```c
/* 1. Punteros de salida */
int dividir(int a, int b, int *cociente, int *resto) {
    if (b == 0) return -1;          /* codigo de error */
    *cociente = a / b;
    *resto    = a % b;
    return 0;                       /* exito */
}

/* 2. Devolver una struct */
struct resultado f(void);
```

El patron 1 es **el de toda la API de UNIX**: se devuelve `0`/`-1` como estado y los datos salen por punteros. Lo vas a ver en todas las llamadas al sistema del resto del curso.

---

## Ejercicios

1. Escribe `void duplica(int *n)` que multiplique por 2 el valor apuntado.
2. Escribe `intercambia(int *a, int *b)` y comprueba que funciona.
3. Define un `struct alumno` con nombre (array), edad y nota. Rellena uno e imprimelo.
4. Haz lo mismo con un puntero a la estructura, usando `->`.
5. Escribe `clone_string()` tu mismo, con `malloc` y `strcpy`.
6. Completa `getopt.c` de la practica y prueba las cuatro invocaciones del enunciado.
