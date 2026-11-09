# SO - C 01 - Del codigo al ejecutable

Parte de [Inicio](../../README.md). Siguiente: [C 02 - Tipos y variables](02-tipos-y-variables.md)

---

## 1. Por que C en Sistemas Operativos

| C | Java |
|---|------|
| Tu gestionas la memoria | recolector de basura |
| No hay valores por defecto | todo se inicializa a 0/null |
| Acceso directo a direcciones de memoria | referencias opacas |
| Los errores se comprueban a mano (valor de retorno) | excepciones |
| Compila a codigo maquina nativo | bytecode + JVM |

Linux esta escrito en C. Si quieres hablar con el kernel, hablas C.

> [!danger] La mentalidad que tienes que cambiar
> En Java, si te equivocas, salta una excepcion. **En C, si te equivocas, el programa a veces funciona igual.** Y luego revienta en otro sitio tres horas despues. Por eso se compila siempre con `-Wall` y se depura con `gdb`.

---

## 2. Un programa minimo

```c
#include <stdio.h>        /* biblioteca de entrada/salida */

int main(void)
{
    printf("Hola mundo\n");
    return 0;             /* 0 = todo fue bien */
}
```

| Elemento | Explicacion |
|----------|-------------|
| `#include <stdio.h>` | directiva del preprocesador. **No lleva `;`** |
| `int main(...)` | punto de entrada. Devuelve el codigo de salida (el `$?` del shell) |
| `printf` | imprime con formato. `\n` es salto de linea |
| `return 0;` | exito. Distinto de 0 = error |

C es **sensible a mayusculas**: `Main` no es `main`.

---

## 3. Las 4 etapas de compilacion (pregunta clasica de examen)

```
  hola.c
    │
    │  1. PREPROCESADO      gcc -E hola.c -o hola.i
    ▼                       expande #include y #define, quita comentarios
  hola.i   (C puro, sin directivas, enorme)
    │
    │  2. COMPILACION       gcc -S hola.i -o hola.s
    ▼                       traduce C -> ensamblador
  hola.s   (ensamblador)
    │
    │  3. ENSAMBLADO        gcc -c hola.s -o hola.o
    ▼                       ensamblador -> codigo maquina
  hola.o   (objeto: binario, pero INCOMPLETO)
    │
    │  4. ENLAZADO          gcc hola.o -o hola   (lo hace 'ld')
    ▼                       une tus .o con las bibliotecas (libc, libm...)
  hola    (ejecutable)
```

```bash
gcc hola.c -o hola          # hace las 4 etapas de golpe
gcc --save-temps hola.c     # las hace todas Y deja los ficheros intermedios
gcc -E hola.c -o hola.i     # solo preprocesar
```

> [!tip] Ejercicio 1.1 de la Practica
> Te piden generar `hello2.i` y mirar que ha pasado con `min()` y con `#include <stdio.h>`.
> - `hello2.i` tendra **miles de lineas**: el `#include` ha pegado ahi dentro todo el contenido de `stdio.h` (y de todo lo que stdio.h incluye a su vez).
> - `min(a,b)` **ha desaparecido**: el preprocesador lo ha sustituido por `( (a<b)?a:b )`. Ya no es una llamada a funcion, es texto pegado en su sitio.

---

## 4. El preprocesador: `#define` y macros

```c
#define N 5                          /* constante */
#define min(x,y) ( (x<y)?x:y )       /* macro con parametros */
```

El preprocesador hace **sustitucion de texto tonta**, no entiende C.

```c
a = min(a,b);
/* se convierte literalmente en: */
a = ( (a<b)?a:b );
```

> [!danger] Por que las macros son peligrosas
> ```c
> #define CUADRADO(x) x*x
> CUADRADO(2+3)   ->  2+3*2+3   ->  11   (queriamos 25)
> ```
> Por eso se escriben **siempre con parentesis**: `#define CUADRADO(x) ((x)*(x))`.
> Y ojo con los efectos secundarios: `min(i++, j)` puede incrementar `i` dos veces.

Otras directivas utiles:

```c
#define DEBUG                    /* definir una marca */
#ifdef DEBUG
    printf("valor de x: %d\n", x);

#ifndef DEFS_H                   /* guardas de inclusion: evitan incluir 2 veces */
#define DEFS_H
/* ... contenido del .h ... */
```

---

## 5. Ficheros `.h` vs `.c`

| Fichero | Contiene | Se compila? |
|---------|----------|-------------|
| `.c` | **implementacion**: el cuerpo de las funciones | Si, genera un `.o` |
| `.h` | **declaraciones**: firmas de funciones, `struct`, `#define`, `typedef` | No. Se pega dentro de los `.c` que lo incluyen |

```c
/* auxiliar.h  -- QUE hay */
int suma(int a, int b);

/* auxiliar.c  -- COMO se hace */
#include "auxiliar.h"
int suma(int a, int b) { return a + b; }

/* init.c -- quien lo usa */
#include "auxiliar.h"
int main(void) { return suma(2,3); }
```

| Sintaxis | Busca en |
|----------|----------|
| `#include <stdio.h>` | directorios del sistema (`/usr/include`) |
| `#include "auxiliar.h"` | **primero el directorio actual**, luego el sistema |

### Declaracion adelantada (*forward declaration*)

```c
int sum(int *arr, int n);     /* DECLARACION: solo la firma, acaba en ';' */

int main(void) { ... sum(v, 3) ... }

int sum(int *arr, int n) {    /* DEFINICION: el cuerpo */
    ...
}
```

Sin la declaracion adelantada, cuando el compilador llega a `sum(v,3)` todavia no sabe que es `sum`, y en C antiguo se la inventaba suponiendo que devuelve `int`. Eso causa errores silenciosos.

> [!note] Ejercicio 1.6 (arg1.c)
> Te piden comentar las declaraciones adelantadas de `sum()` y `sumC()` y recompilar. Con `-Wall` veras un aviso tipo *"implicit declaration of function"*. Ese aviso es en realidad un error grave disfrazado.

---

## 6. Los avisos del compilador son tu mejor amigo

```bash
gcc -Wall -g hola.c -o hola
```

| Flag | Para que |
|------|----------|
| `-Wall` | **activa todos los avisos.** Usalo SIEMPRE |
| `-Wextra` | aun mas avisos |
| `-g` | mete informacion de depuracion (necesario para `gdb`) |
| `-o nombre` | nombre del ejecutable (por defecto `a.out`) |
| `-c` | solo compilar, no enlazar (genera `.o`) |
| `-lm` | enlazar con la biblioteca matematica (`libm`) |
| `-O2` | optimizar |

> [!warning] `-w` minuscula NO es `-Wall`
> `-w` (minuscula) **desactiva** todos los avisos. El enunciado de la P1 usa `gcc -g -w -o primes primes.c` justamente para que no veas las pistas y tengas que encontrar el error con `gdb`. Para tu codigo del dia a dia usa `-Wall`.

### Bibliotecas: `-lm`

Si usas `sqrt()`, `pow()`, etc. de `<math.h>`, el `#include` le da al **compilador** la firma de la funcion, pero el codigo de la funcion esta en `libm`. Hay que decirle al **enlazador** donde buscarlo:

```bash
gcc calc.c -o calc -lm       # -l + 'm'  =  libm
```

Si te olvidas, el error no es de compilacion sino **de enlazado**:

```
undefined reference to `sqrt'
```

> [!tip] Ejercicio 1.2 de la Practica
> Te piden comentar `LIBS = -lm` del makefile y recompilar. El error que sale es `undefined reference`, y la etapa que falla es **el enlazado**, no la compilacion. Esa es la respuesta que buscan.

---

## Ejercicios

1. Escribe `hola.c`, compilalo con `-Wall` y ejecutalo. Mira su codigo de salida con `echo $?`.
2. Genera `hola.i` con `-E` y cuenta cuantas lineas tiene (`wc -l`). Compara con las de `hola.c`.
3. Compila `hello2.c` de la practica y explica por que `a` se queda en 9.
4. Escribe la macro `CUADRADO` mal (sin parentesis) y comprueba que `CUADRADO(2+3)` da 11.
5. Separa un programa en `main.c` + `aux.c` + `aux.h` y compilalo en dos pasos (`-c` y luego enlazar).
