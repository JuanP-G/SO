# SO - P1 Ejercicio 1 - Runbook

Parte de [Inicio](../../README.md). Teoria de apoyo: [C 01 - Del codigo al ejecutable](../../fundamentos/c/01-del-codigo-al-ejecutable.md) · [C 04 - Punteros](../../fundamentos/c/04-punteros.md) · [Herramientas - gcc make gdb](../../fundamentos/herramientas-gcc-make-gdb.md)

> Comandos y salidas **verificados ejecutandolos**. Los "fallos" que veras son intencionados: son el ejercicio.

---

## 0. Preparar

```bash
mkdir -p ~/so/p1 && cd ~/so/p1
tar -xzvf ~/Descargas/ficheros_p1.tar.gz
cd ejercicio1
```

---

## 1. Compilacion (`hello2.c`)

```bash
cd 1-Compilacion
gcc -Wall hello2.c -o hello2 && ./hello2
gcc -E hello2.c -o hello2.i        # solo preprocesar
wc -l hello2.c hello2.i
tail -20 hello2.i
```

**Salida de `./hello2`:**

```
Hello world 	 a= 7 b= 9
Hello world 	 a= 8 b= 9
Hello world 	 a= 9 b= 9
Hello world 	 a= 9 b= 9
Hello world 	 a= 9 b= 9
```

**`wc -l`:** `hello2.c` = 19 lineas, `hello2.i` = **832 lineas**.

### Respuestas

| Pregunta | Respuesta |
|---|---|
| Que ha ocurrido con `min(a,b)` | **Ha desaparecido.** El preprocesador lo sustituyo por su texto: `a = ( (a<b)?a:b );`. Ya no es una llamada a funcion, es codigo pegado en el sitio |
| Que efecto ha tenido `#include <stdio.h>` | Ha pegado **el contenido entero** de `stdio.h` (y de todo lo que este incluye a su vez) al principio del fichero: de 19 a 832 lineas |

Lo que ves al final del `.i`:

```c
int a = 7;
int b = 9;
int main() {
 char* cad = "Hello world";
 int i;
 for (i=0;i<5;i++) {          /* N -> 5 */
   printf("%s \t a= %d b= %d\n",cad,a,b);
   a++;
   a = ( (a<b)?a:b );         /* min(a,b) expandido */
 }
 return 0;
}
```

> [!note] Por que `a` se queda en 9
> `a++` lo sube, y `min(a,b)` lo vuelve a bajar a `b`=9 en cuanto lo supera. A partir de la 3a vuelta se queda clavado.

---

## 2. Make

Ver [Herramientas - gcc make gdb](../../fundamentos/herramientas-gcc-make-gdb.md), que tiene las respuestas completas con las salidas reales. Resumen de los comandos:

```bash
cd ../2-Make
make                    # 1a vez: compila los .o y enlaza
make                    # 2a vez: SOLO reenlaza (ojo, no dice "Nothing to be done")
touch auxiliar.c && make   # recompila solo auxiliar.o
make clean

# prueba de .PHONY
touch clean && make clean          # funciona, porque clean esta en .PHONY
# comenta la linea .PHONY: clean en el makefile y repite:
touch clean && make clean          # "make: 'clean' is up to date."  -> NO borra

# prueba de -lm
# comenta  LIBS = -lm  en el makefile
make clean && make      # undefined reference to 'sin' / 'cos'  -> falla el ENLAZADO
```

---

## 3. Tamanos de variables

```bash
cd ../3-Tams
gcc -w main1.c -o main1 && ./main1
gcc -Wall main2.c -o main2      # <-- NO COMPILA
```

**`./main1`:**

```
a = 122 a = z 
a = -128 a = ? b=41  b=)
Size of int: 4      Size of char: 1     Size of float: 4
Size of double: 8   Size of long: 8     Size of short: 2
Size of void*: 8
```

### Respuestas de `main1.c`

| Pregunta | Respuesta |
|---|---|
| Por que `%d` y `%c` dan cosas distintas para 'a' | Son los **mismos bits** interpretados de dos formas: `%d` como numero, `%c` como codigo ASCII. Un `char` es un entero de 1 byte, no "una letra" |
| Cuanto ocupa un `char` | **1 byte** |
| Por que 122 + 6 da -128 | **Desbordamiento.** Un `char` con signo va de -128 a 127. `122+6 = 128 = 1000 0000`, que en complemento a dos vale **-128**. Da la vuelta al otro extremo del rango |
| `long` y `double` ocupan 8 bytes, por que 2 tipos | El tamano es lo unico que comparten. `long` es complemento a dos (**enteros exactos**, hasta ~9.2e18); `double` es IEEE-754 con mantisa y exponente (**valores aproximados**, hasta ~1e308). Ni el rango ni la precision ni la codificacion son iguales |

### `main2.c`: problema de compilacion, no de ejecucion

```
main2.c:13:5: error: variably modified 'array2' at file scope
   13 | int array2[a];
```

**Por que:** `int array2[a];` es una variable **global**, y el tamano de una global tiene que conocerse en **tiempo de compilacion**. `a` es una variable, no una constante.

**Solucion:** usar una constante (`#define` o un literal), o mover el array dentro de `main()` (ahi si se permite un VLA en C99):

```c
int array2[7];          /* o  #define A 7  ->  int array2[A]; */
```

### Respuestas de `main2.c` (tras arreglarlo)

| Pregunta | Respuesta |
|---|---|
| Que significa `%lu` en printf | *unsigned long*. Es el formato de `size_t`, que es lo que devuelve `sizeof()` |
| A que direccion apunta `pc` | A `&x`, que es una de las globales declaradas antes. **Si coincide.** Los tamanos NO: `pc` mide 8 (es un puntero), `x` mide 1 (es un char) |
| Coincide `sizeof(array1)` con el numero de elementos | **No.** `sizeof` da 40 **bytes**; elementos hay 10. Cada `int` ocupa 4 |
| Coinciden las direcciones de `str1` y `str2` | **No.** `str1` apunta a un literal en la zona de solo lectura; `str2` es un array propio en otra zona |
| Por que los `sizeof()` de `str1` y `str2` son distintos | `str1` es un **puntero** -> 8 bytes. `str2` es un **array** con su copia -> 21 bytes (20 letras + `\0`). Ver [C 03 - Arrays y strings](../../fundamentos/c/03-arrays-y-strings.md) |

---

## 4. Arrays

```bash
cd ../4-Arrays
gcc -Wall array1.c -o array1 && ./array1
gcc -Wall array2.c -o array2 && ./array2
```

**`./array1`** (con `-Wall` avisa: *'sizeof' on array function parameter will return size of 'int \*'*):

```
Address of list 0x7ffc...390  Address of list[0]: 0x7ffc...390  Address of list[1]: 0x7ffc...394.  Sizeof list 20
Address of the array: 0x7ffc...390  Sizeof array 8      <- init_array
Address of the array: 0x7ffc...390  Sizeof array 8      <- init_array2
```

> [!important] Toda la respuesta del apartado esta en esas 3 lineas
> **La direccion es la misma en los tres sitios, pero el `sizeof` pasa de 20 a 8.** El array no se ha copiado: solo ha viajado su direccion. Y `&list[1]` = `...394` (no `...391`) confirma que cada `int` ocupa 4 bytes.

### Degradacion del array a puntero (*array decay*)

| Donde | Que es realmente | `sizeof` |
|---|---|---|
| en `main` | un **array** de 5 ints | **20** = 5 x 4 bytes |
| dentro de cualquiera de las 2 funciones | un **puntero** `int*` | **8** = una direccion en 64 bits |

Aunque escribas el parametro como `int array[]` o `int array[N]`, **C lo convierte silenciosamente en `int *array`**.

### Respuestas de `array1.c`

| Pregunta | Respuesta |
|---|---|
| Por que no hace falta `&list` | Porque **el nombre del array YA es la direccion** de su primer elemento: `list == &list[0]` |
| Que hay en la direccion de `list` | El primer elemento, `list[0]` |
| Por que hay que pasar el tamano a `init_array` | Porque al pasar un array a una funcion **se degrada a puntero**. Dentro, `sizeof` mide el puntero, no el array. La funcion no tiene forma de saber cuantos elementos hay |
| Por que `sizeof()` da 8 dentro y 20 en `main` | 20 = el array real (5 ints x 4 bytes). 8 = el tamano de un `int*` en 64 bits |
| Por que `init_array2` NO necesita el tamano | Porque usa directamente la constante `N` del `#define`, conocida en tiempo de compilacion y visible en todo el fichero. **NO** es porque `sizeof` funcione ahi. El precio: solo sirve para arrays de exactamente `N` elementos |
| Coincide el `sizeof` de `init_array2` con el de `main` | **No: sigue dando 8.** Aunque el parametro se escriba `int array[N]` con el tamano explicito, **C ignora ese `N`** y degrada a `int*` igual que en `init_array`. `void f(int a[])`, `void f(int a[5])` y `void f(int *a)` son la MISMA declaracion para el compilador |

### `array2.c`: la copia NO funciona

```
-------------------
0 0 0 0 0 0 1419770608 32615 -849523216 32767 
```

Esperabas `4 3 8 5 6 9 0 1 7 2`. Sale basura porque:

```c
void copyArray(int src[], int dst[], int size)
{
    dst = src;          /* solo cambia la COPIA LOCAL del puntero dst */
}                       /* al salir de la funcion, no ha pasado nada */
```

`dst` es un puntero pasado **por valor**. Reasignarlo dentro de la funcion no toca el array de `main`. Lo que imprime son los valores sin inicializar de `B`.

**Version correcta:**

```c
void copyArray(int src[], int dst[], int size)
{
    int i;
    for (i = 0; i < size; i++)
        dst[i] = src[i];       /* copiar CONTENIDO, no reasignar el puntero */
}
```

(o `memcpy(dst, src, size * sizeof(int));` con `<string.h>`)

---

## 5. Punteros

```bash
cd ../5-punteros
for f in punteros1 punteros2 punteros3; do
    gcc -Wall -g $f.c -o $f && ./$f; echo "  -> exit=$?"
done
```

| Programa | Resultado | Codigo de salida |
|---|---|---|
| `punteros1` | **Segmentation fault** | **139** |
| `punteros2` | imprime 4 lineas, termina "bien" | 0 |
| `punteros3` | `free(): invalid pointer` + **Aborted** | **134** |

Las respuestas completas de los tres estan en [C 04 - Punteros](../../fundamentos/c/04-punteros.md), secciones 3, 5 y 6.

Resumen rapido:

| | Error | Compilacion o ejecucion? |
|---|---|---|
| `punteros1` | `ptr = (int*)0x600a48` y luego `*ptr = 13` -> escribir en una direccion inventada | **Ejecucion** (SIGSEGV) |
| `punteros2` | `free(ptr)` y despues `*ptr = 3` -> **use after free** | Ejecucion, pero **silencioso**: parece funcionar |
| `punteros3` | `ptr = &c` pierde la zona de `malloc` (**fuga**), y luego `free(ptr)` intenta liberar una global | **Ejecucion** (abort) |

> [!tip] Comprobacion con valgrind
> ```bash
> valgrind --leak-check=full ./punteros3
> ```
> Te senala exactamente la fuga y el `free` invalido.

---

## 6. Funciones

```bash
cd ../6-Funciones
gcc -Wall arg1.c -o arg1 && ./arg1
gcc -Wall arg2.c -o arg2 && ./arg2
```

**`./arg1`:**

```
Complex numbers addition. (1.000000,2.000000 i) + (3.000000,1.000000 i) =(4.000000,3.000000 i)
Integer addition:  x +y = 4 + 5 = 9 
```

### Respuestas

| Pregunta | Respuesta |
|---|---|
| Por que `xc` no se modifica tras `sumC` | **Paso por valor.** `sumC` recibe una copia; la modifica en su propio marco de pila y la copia muere al volver. Para que se viera fuera habria que pasar `&xc` y recibir un puntero |
| Donde se modifica esa informacion | En la **pila**, dentro del marco de `sumC`, que se destruye al terminar la funcion |
| Que ocurre al comentar las declaraciones adelantadas | Con `-Wall`: *implicit declaration of function*. El compilador no conoce la firma y se inventa una (asume que devuelve `int`). En C moderno eso es error; si compila, el resultado es impredecible cuando la funcion devuelve otra cosa |
| Por que a veces `.` y a veces `->` | `.` cuando tienes **la estructura**; `->` cuando tienes **un puntero** a ella. `p->campo` es `(*p).campo` |
| Por que `zc` pasa a ser incorrecto | `arg2.c` avisa: *function returns address of local variable*. La funcion devuelve la direccion de una variable local, que **muere al salir**. `zc` apunta a memoria destruida y su contenido se corrompe en cuanto se reutiliza la pila |
| Como corregirlo | Devolver la estructura **por valor**, o reservarla con `malloc` para que sobreviva (y liberarla despues) |

---

## 7. Cadenas de caracteres

```bash
cd ../7-Strings
gcc -Wall strings1.c -o strings1 && ./strings1
gcc -Wall strings2.c -o strings2 && ./strings2
```

**`./strings1`:**

```
msg: , p: Bonjour, msg2: Hello
dir de msg: 0x7ffc...806, dir de p: 0x5607...008, dir de msg2: 0x7ffc...810
msg: , p: Hello, msg2: Hello
dir de msg: 0x7ffc...806, dir de p: 0x7ffc...810, dir de msg2: 0x7ffc...810
msg: , p: Hi, msg2: Hi
msg len: 0 p len 2 msg2 len 2
msg size: 10 p size 8 msg2 size 28
msg: By, p: Hi, msg2: Hi
```

> [!important] Fijate en las direcciones de la 2a y 4a linea
> Al principio `p` esta en `0x5607...` (zona de solo lectura del ejecutable, donde viven los literales) y `msg`/`msg2` en `0x7ffc...` (la **pila**). Tras `p = msg2`, `p` pasa a valer exactamente la direccion de `msg2`. Ahi se ve en vivo el mapa de memoria de [C 05 - Memoria dinamica](../../fundamentos/c/05-memoria-dinamica.md).

### El error de `strings1.c`

```c
char msg[10];                              /* SIN inicializar -> basura, sin '\0' */
printf("msg: %s, ...", msg, ...);          /* <-- AQUI */
```

**Se manifiesta en ejecucion**, no en compilacion. `printf("%s")` recorre la memoria hasta encontrar un `'\0'`; como `msg` nunca se inicializo, no hay garantia de que exista. Es **comportamiento indefinido**: puede imprimir basura, puede salir vacio (que es lo que pasa aqui, porque la pila estaba a cero por casualidad) o puede petar.

Lo mismo al final: `msg[0]='B', msg[1]='y';` escribe 2 caracteres pero **no pone el terminador** en `msg[2]`.

**Solucion:** comentar los `printf` que usan `msg`, o inicializarlo: `char msg[10] = "";`

> [!warning] El peor tipo de bug
> Que "funcione" no significa que este bien. Este programa da una salida razonable por pura suerte. En otra maquina, u otro dia, imprime basura.

### Respuestas

| Pregunta | Respuesta |
|---|---|
| El error, compilacion o ejecucion? | **Ejecucion.** Compila sin problemas (con `-Wall` puede que ni avise). Es uso de memoria sin inicializar |
| Direccion de la 'B' de "Bonjour" | La que imprime `p` en la primera linea. La `'j'` esta en esa direccion **+ 3** (`p+3`): cada `char` ocupa 1 byte |
| Tras `p = msg2`, como recuperar la direccion de "Bonjour" | Perdiste la referencia, pero el literal sigue en el ejecutable: basta con volver a escribir `p = "Bonjour";`. El compilador **reutiliza el mismo literal** (comprobado: dos `char*` a `"Bonjour"` dan la misma direccion). Lo limpio habria sido guardarla antes en otra variable |
| Por que `p` y `msg2` miden 2, si se asignaron 3 bytes | Los 3 bytes son `'H'`, `'i'` y `'\0'`. `strlen()` cuenta **hasta** el `'\0'` sin incluirlo: **2**. Y valen lo mismo porque `p` y `msg2` apuntan al mismo sitio |
| Por que `strlen()` y `sizeof()` dan distinto | `strlen()` es una **funcion**: recorre y cuenta caracteres hasta el `'\0'`, en **ejecucion**. `sizeof()` es un **operador**: da los bytes del tipo, en **compilacion**. Por eso `msg`=10, `msg2`=28 (lo declarado) y `p`=8 (mide el **puntero**, no la cadena) |

---

**`./strings2`:**

```
str1 original str2 other
str1 original str3 ORIGINAL
```

### Respuestas

| Pregunta | Respuesta |
|---|---|
| Por que `copy` no funciona | `void copy(char* org, char* dst) { dst = org; }` reasigna la **copia local** del puntero. El `str2` de `main` no se entera. Mismo error que `copyArray` del apartado 4 |
| Funciona `copy2`? | **Si.** Recibe `char** dst`, asi que `*dst = org` escribe en la variable `str2` de `main`. Comprobado: pasa a imprimir `str2 original` |
| Implementacion correcta | Depende de que quieras. Ver abajo |
| Que hace `mod()` y por que funciona | Resta **32** a cada caracter, que en ASCII convierte minusculas en mayusculas (`'a'`=97, `'A'`=65). Funciona porque `dst` apunta a `str3`, que es un **array con memoria propia**, y escribir en `dst[i]` toca esa memoria de verdad |

### Implementacion correcta de la copia

Hay dos lecturas del enunciado:

**a) Reapuntar el puntero** (lo que hace `copy2`): necesitas un nivel mas de indireccion.

```c
void copy2(char *org, char **dst) { *dst = org; }
copy2(str1, &str2);        /* str2 pasa a apuntar al mismo literal */
```

**b) Copiar el contenido de verdad**: hace falta un destino **con memoria propia**.

```c
void copy(const char *org, char *dst) {
    int i = 0;
    while ((dst[i] = org[i]) != '\0') i++;    /* copia incluido el '\0' */
}

char buffer[32];
copy(str1, buffer);        /* equivale a strcpy(buffer, str1) */
```

> [!danger] `copy(str1, str2)` no se puede arreglar copiando contenido
> `str2` es un `char*` que apunta a un **literal de solo lectura**. Escribir ahi es segmentation fault. Por eso el ejercicio esta montado con `copy2` y con `str3` como array: son las dos unicas formas que funcionan.

> [!warning] `mod()` tiene un bug que el enunciado no menciona
> Copia `strlen(org)` caracteres pero **nunca escribe el `'\0'`**. Que `printf` imprima `ORIGINAL` limpio es suerte: `str3` estaba a ceros. Arreglo: `dst[i] = '\0';` despues del bucle.

---

## Checklist

- [ ] 1-Compilacion: generado `hello2.i`, visto `min()` expandido
- [ ] 2-Make: las 5 pruebas del makefile
- [ ] 3-Tams: `main1` ejecutado, `main2` arreglado y ejecutado
- [ ] 4-Arrays: `copyArray` corregido
- [ ] 5-punteros: los 3 ejecutados, errores identificados, pasado `valgrind`
- [ ] 6-Funciones: comentadas las declaraciones adelantadas y visto el aviso
- [ ] 7-Strings: `strings1` arreglado, `copy` reimplementado
