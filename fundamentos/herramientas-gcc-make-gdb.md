# SO - Herramientas - gcc make gdb

Parte de [Inicio](../README.md). Anterior: [C 06 - Funciones y structs](c/06-funciones-y-structs.md)

---

## 1. `gcc` - chuleta

```bash
gcc -Wall -g hola.c -o hola      # el comando del dia a dia
./hola
```

| Flag | Efecto |
|------|--------|
| `-o nombre` | nombre del ejecutable (por defecto `a.out`) |
| `-Wall` | **todos los avisos.** No lo quites nunca |
| `-Wextra` | aun mas |
| `-Werror` | trata los avisos como errores |
| `-g` | informacion de depuracion, para `gdb` |
| `-c` | solo compilar, generar `.o`, no enlazar |
| `-E` | solo preprocesar |
| `-S` | hasta ensamblador |
| `--save-temps` | deja los ficheros intermedios `.i`, `.s`, `.o` |
| `-lm` | enlazar con `libm` (`sqrt`, `pow`...) |
| `-I dir` | buscar cabeceras tambien en `dir` |
| `-O0` .. `-O3` | nivel de optimizacion (usa `-O0` al depurar) |
| `-w` | **desactiva** los avisos (solo si el enunciado te lo pide) |

Compilacion separada:

```bash
gcc -Wall -g -c auxiliar.c -o auxiliar.o
gcc -Wall -g -c init.c     -o init.o
gcc auxiliar.o init.o -o prueba -lm      # enlazado
```

### Leer los errores

| Mensaje | Que pasa |
|---------|----------|
| `undefined reference to 'sqrt'` | error de **enlazado**: falta `-lm` o un `.o` |
| `implicit declaration of function 'f'` | falta el `#include` o la declaracion adelantada |
| `expected ';' before ...` | casi siempre el `;` falta en la linea **anterior** |
| `control reaches end of non-void function` | falta el `return` |
| `assignment makes pointer from integer` | estas mezclando enteros y punteros |
| `segmentation fault` | **no es del compilador**: peta en ejecucion. A `gdb`. |

> [!tip] Arregla siempre el PRIMER error
> Un `;` que falta genera 30 errores en cascada. Corrige el de arriba y recompila.

---

## 2. `make`

`make` recompila **solo lo que ha cambiado**, comparando **fechas de modificacion**.

### Anatomia de un Makefile

```makefile
CC = gcc                       # variables
CFLAGS = -Wall -g
LIBS = -lm

%.o: %.c                       # regla generica: como sacar un .o de un .c
	$(CC) $(CFLAGS) -c -o $@ $<

all : auxiliar.o init.o auxiliar.h
	gcc $(CFLAGS) -o prueba auxiliar.o init.o $(LIBS)

.PHONY: clean

clean:
	-rm *.o prueba
```

```
objetivo : dependencias
<TAB>receta
```

> [!danger] La receta va con TABULADOR, no con espacios
> Es el error mas absurdo de make. Si pones espacios: `Makefile:5: *** missing separator. Stop.`

### Variables automaticas

| Variable | Significa |
|----------|-----------|
| `$@` | el **objetivo** (lo que hay a la izquierda de los `:`) |
| `$<` | la **primera dependencia** |
| `$^` | **todas** las dependencias |

En `$(CC) $(CFLAGS) -c -o $@ $<`, para `auxiliar.o` se expande a
`gcc -Wall -g -c -o auxiliar.o auxiliar.c`.

### Como decide make si recompilar

```
Si  fecha(dependencia) > fecha(objetivo)  ->  ejecuta la receta
Si no                                     ->  "make: Nothing to be done"
```

Por eso `touch aux.c` fuerza la recompilacion: le pone la fecha actual, que es posterior a la de `aux.o`.

### Respuestas al ejercicio 1.2 de la Practica

| Pregunta | Respuesta |
|----------|-----------|
| Variables definidas | `CC`, `CFLAGS`, `LIBS` |
| Objetivos (*targets*) | `%.o`, `all`, `clean` |
| Que hace `make` la 2a vez | **NO recompila los `.o`** (son mas nuevos que los `.c`), pero **si vuelve a hacer el enlazado**. Mira la salida real abajo y fijate en por que |
| Que hace `touch auxiliar.c` | le pone la fecha actual -> make lo ve "modificado" -> recompila **solo** `auxiliar.o` (no `init.o`) y reenlaza |
| Que hace `make clean` | ejecuta `rm *.o prueba` |
| Por que `clean` es `.PHONY` | porque `clean` **no es un fichero**. Si existiera un fichero llamado `clean`, make veria que "ya esta hecho" y no ejecutaria la receta. `.PHONY` le dice "esto es una accion, no un fichero" |
| Que pasa al comentar `LIBS = -lm` | `undefined reference to 'sin'` / `'cos'`. Falla el **enlazado**, no la compilacion: los `.o` se generan perfectamente, lo que falla es unirlos con `libm` |

### Salida real, paso a paso

```
$ make                                       # 1a vez: lo hace todo
gcc -Wall -g -c -o auxiliar.o auxiliar.c
gcc -Wall -g -c -o init.o init.c
gcc -Wall -g -o prueba auxiliar.o init.o -lm

$ make                                       # 2a vez
gcc -Wall -g -o prueba auxiliar.o init.o -lm    <-- solo reenlaza
```

> [!important] Por que la 2a vez SI reenlaza
> Porque el objetivo se llama **`all`**, y **no existe ningun fichero llamado `all`**. Como make no encuentra el fichero, asume que hay que construirlo siempre. En cambio los `.o` si son ficheros reales y make compara sus fechas, por eso esos no se recompilan.
>
> Si el objetivo se llamara `prueba` (el ejecutable de verdad), la segunda vez diria `make: 'prueba' is up to date.` Dicho de otro modo: **este makefile tiene el fallo de que `all` deberia estar en `.PHONY` tambien**, y lo correcto seria que la regla construyese el fichero `prueba`.

```
$ touch auxiliar.c && make                   # solo lo afectado
gcc -Wall -g -c -o auxiliar.o auxiliar.c
gcc -Wall -g -o prueba auxiliar.o init.o -lm
```

```
$ make clean
rm *.o prueba
```

### Comprobacion de `.PHONY`

```
# con  .PHONY: clean  activo, aunque exista un fichero 'clean':
$ touch clean && make clean
rm *.o prueba                                <-- funciona

# comentando la linea  #.PHONY: clean :
$ touch clean && make clean
make: 'clean' is up to date.                 <-- NO borra nada
```

Ahi tienes la demostracion: sin `.PHONY`, un fichero llamado `clean` deja el objetivo inutilizado.

El `-` delante de `-rm *.o prueba` significa **"sigue aunque falle"**. Asi `make clean` no da error si no hay `.o` que borrar.

### Ordenes utiles

```bash
make              # construye el primer objetivo del fichero
make clean        # ejecuta ese objetivo
make all
make -n           # muestra lo que haria SIN hacerlo   <-- muy util
make -B           # reconstruir todo a la fuerza
```

---

## 3. `gdb` - el depurador (ejercicio 2 de la Practica)

```bash
gcc -g -Wall primes.c -o primes     # -g es IMPRESCINDIBLE
gdb ./primes
```

### Ordenes basicas

| Orden | Abrev. | Que hace |
|-------|--------|----------|
| `run` | `r` | ejecutar |
| `run 5 hola` | | ejecutar con argumentos |
| `break main` | `b main` | poner un punto de ruptura en una funcion |
| `break primes.c:42` | `b 42` | punto de ruptura en una linea |
| `next` | `n` | siguiente linea, **sin entrar** en las funciones |
| `step` | `s` | siguiente linea, **entrando** en las funciones |
| `continue` | `c` | seguir hasta el siguiente breakpoint |
| `finish` | | terminar la funcion actual y volver |
| `print x` | `p x` | ver el valor de una variable |
| `print *ptr` | | ver el contenido apuntado |
| `print arr[3]` | | un elemento |
| `print &x` | | su direccion |
| `display x` | | mostrar `x` en cada parada, automaticamente |
| `info locals` | | todas las variables locales |
| `info args` | | los argumentos de la funcion |
| `backtrace` | `bt` | **pila de llamadas: donde estoy y como llegue** |
| `list` | `l` | ver el codigo fuente alrededor |
| `watch x` | | parar cuando `x` cambie de valor |
| `quit` | `q` | salir |

### La sesion tipica cuando algo peta

```
$ gdb ./primes
(gdb) run
Program received signal SIGSEGV, Segmentation fault.
0x... in compute_primes (result=0x0, n=10) at primes.c:45
45          result[i] = x;

(gdb) bt              <-- de donde venia
(gdb) print i         <-- que valia el indice
(gdb) print n
(gdb) print result    <-- 0x0  ->  el puntero es NULL!
```

En tres ordenes sabes que `result` llego a `NULL` y por que.

> [!tip] `bt` y `print` resuelven el 90% de los casos
> No hace falta saberse `gdb` entero. Ejecuta, mira donde peta con `bt`, e imprime las variables sospechosas con `p`.

### Pistas para el ejercicio 2 (`primes.c`)

El programa debe calcular la suma de los `n` primeros primos. Tiene **varios** errores. Busca en estos cuatro sitios:

| Zona | Que mirar |
|------|-----------|
| `main`, linea del `if` | `if (argc = 2)` -> es una **asignacion**, no una comparacion. La condicion es siempre cierta |
| `main`, uso de `argv` | `argv[2]` con solo 2 argumentos esta fuera de rango, y el resultado de `atoi` no se asigna a `n` |
| `sum()` | `int total;` sin inicializar. Que vale al empezar? |
| `compute_primes()` | empieza en `x = 2` y hace `x += 2` **solo dentro del `if`**. Que pasa si `is_prime(2)` es falso? |
| `is_prime()` | lo primero que hace es `if (x % 2 == 0) return 0;`. Que devuelve para `x == 2`? |

> [!warning] Sintoma que veras al ejecutarlo
> **El programa se queda colgado para siempre.** No peta, no imprime: bucle infinito.
> Razon: `is_prime(2)` devuelve 0 (porque 2 es par), asi que el `if` de `compute_primes` nunca se cumple, `x` nunca se incrementa y el `while` gira eternamente con `x = 2`.
> Para pararlo, `Ctrl + C`. En `gdb`, `Ctrl + C` te deja dentro y puedes hacer `bt`, `print x`, `print i` y verlo al instante.

Ponle un `break compute_primes`, haz `next` unas cuantas veces y ve mirando `print x` y `print i`. Se ve enseguida.

> [!note] Compilar para depurar
> El enunciado dice `gcc -g -w -o primes primes.c`. El `-w` esconde los avisos a proposito, para obligarte a usar `gdb`. Cuando acabes, recompila con `-Wall` y compara: veras que gcc te habria cantado la mitad de los fallos el solo.

---

## 4. Flujo de trabajo recomendado

```bash
cd ~/so/p1/alumnos/ejercicio3
make                          # o: gcc -Wall -g getopt.c -o getopt
./getopt -l 4 -e hola2        # probar
gdb ./getopt                  # si peta
make clean                    # limpiar
```

Y el alias que mas vas a agradecer, en tu `~/.bashrc`:

```bash
alias gccw='gcc -Wall -Wextra -g'
```

Luego `source ~/.bashrc` y ya puedes hacer `gccw hola.c -o hola`.

---

## Ejercicios

1. Compila un programa en los 4 pasos por separado y mira los ficheros intermedios.
2. En el ejercicio 1.2, ejecuta `make`, luego `make` otra vez, luego `touch auxiliar.c` y `make`. Explica cada salida.
3. Comenta `.PHONY: clean`, haz `touch clean` y prueba `make clean`.
4. Comenta `LIBS = -lm`, haz `make clean && make` y explica el error y la etapa.
5. Depura `primes.c` con `gdb` hasta encontrar los errores.
6. Recompila `primes.c` con `-Wall` y compara los avisos con lo que encontraste.
