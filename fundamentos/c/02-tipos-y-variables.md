# SO - C 02 - Tipos y variables

Parte de [Inicio](../../README.md). Anterior: [C 01 - Del codigo al ejecutable](01-del-codigo-al-ejecutable.md) | Siguiente: [C 03 - Arrays y strings](03-arrays-y-strings.md)

---

## 1. Tipos basicos (en Linux 64 bits)

| Tipo | Bytes | Rango | `printf` |
|------|-------|-------|----------|
| `char` | **1** | -128 .. 127 | `%c` (caracter) o `%d` (numero) |
| `unsigned char` | 1 | 0 .. 255 | `%u` |
| `short` | 2 | -32768 .. 32767 | `%hd` |
| `int` | **4** | -2^31 .. 2^31-1 | `%d` |
| `unsigned int` | 4 | 0 .. 2^32-1 | `%u` |
| `long` | **8** | -2^63 .. 2^63-1 | `%ld` |
| `unsigned long` | 8 | 0 .. 2^64-1 | **`%lu`** |
| `float` | 4 | ~7 digitos significativos | `%f` |
| `double` | **8** | ~15 digitos significativos | `%lf` |
| `void*` (puntero) | **8** | una direccion de memoria | `%p` |

> [!important] Los tamanos NO estan fijados por el estandar
> C solo garantiza `char <= short <= int <= long`. Los valores de arriba son los de x86-64 con Linux. En un microcontrolador de 16 bits un `int` puede ocupar 2 bytes. **Por eso existe `sizeof()`**: para no tener que suponer.

```c
printf("Size of int: %lu\n", sizeof(int));   /* sizeof devuelve size_t -> %lu */
```

### Respuestas al ejercicio 1.3 de la Practica

**`long` y `double` ocupan lo mismo (8 bytes), por que hay dos tipos?**
Porque el **tamano** es lo unico que comparten. La **codificacion interna** es completamente distinta: `long` es complemento a dos (enteros exactos), `double` es IEEE-754 (mantisa + exponente, valores aproximados con rango enorme). Un `double` llega a 10^308 pero no puede representar exactamente 0.1; un `long` es exacto siempre pero se queda en ~9.2*10^18.

---

## 2. Overflow: por que `122 + 6` da `-128`

Este es el `main1.c` de la practica.

```c
char a = 122;          /* la letra 'z' */
printf("%d %c\n", a, a);   /*  122   z  */
a += 6;
printf("%d %c\n", a, a);   /* -128   (basura)  <-- ???  */
```

Un `char` con signo tiene 8 bits y va de -128 a 127.

```
 122  =  0111 1010
+  6
 128  =  1000 0000
```

Pero en complemento a dos, `1000 0000` **no es 128**, es **-128**. Se ha desbordado y ha "dado la vuelta" al otro extremo del rango.

```
  ...126, 127, -128, -127, -126...
           ^     ^
           |     └── 128 cae aqui
           └── maximo de char
```

Los mismos 8 bits, interpretados de dos maneras. Este es el nucleo de la pregunta.

### `%d` vs `%c` sobre la misma variable

```c
char a = 122;
printf("%d", a);   /* 122  -> interpreta los bits como numero */
printf("%c", a);   /* z    -> interpreta los bits como codigo ASCII */
```

**El `char` no es "una letra". Es un entero de 1 byte.** Lo que cambia es como lo imprimes. `'A'` es exactamente `65`, y `'A' + 1 == 'B'`.

| Caracter | ASCII |
|----------|-------|
| `'0'` | 48 |
| `'A'` | 65 |
| `'a'` | 97 |
| `'z'` | 122 |
| `'\0'` | 0 |

---

## 3. `printf` y sus formatos

```c
printf("cadena de formato", arg1, arg2, ...);
```

| Formato | Tipo | Ejemplo |
|---------|------|---------|
| `%d` / `%i` | int | `printf("%d", 42)` |
| `%u` | unsigned int | |
| `%ld` | long | |
| **`%lu`** | unsigned long / `size_t` | **el de `sizeof()`** |
| `%c` | char como caracter | |
| `%s` | cadena (char*) | |
| `%f` | float / double | |
| `%lf` | double | |
| `%p` | puntero (direccion en hexadecimal) | |
| `%x` | entero en hexadecimal | |
| `%%` | un `%` literal | |

Anchura y alineacion:

```c
printf("%5d\n",   42);      /* "   42"  ancho 5, a la derecha */
printf("%-5d|\n", 42);      /* "42   |" a la izquierda */
printf("%.2f\n",  3.14159); /* "3.14"   2 decimales */
printf("%-10s%d\n", "juan", 1000);
```

> [!danger] `printf` no comprueba nada
> Si escribes `printf("%d", 3.5)` o pasas menos argumentos que formatos, **compila sin error** y en ejecucion imprime basura o revienta. Con `-Wall` gcc te avisa. Es otra razon mas para usar `-Wall` siempre.

### `scanf` - leer

```c
int n;
scanf("%d", &n);       /* OJO: el & es OBLIGATORIO */
```

`scanf` necesita saber **donde** guardar el dato, asi que se le pasa la *direccion* de la variable, no su valor. Ver [C 04 - Punteros](04-punteros.md).

Excepcion: con `%s` sobre un array de char no se pone `&`, porque el nombre del array ya es una direccion.

---

## 4. Variables: donde viven y con que valor nacen

```c
int global;              /* variable GLOBAL: fuera de toda funcion */

int main(void) {
    int local;           /* variable LOCAL */
    static int persist;  /* local pero sobrevive entre llamadas */
}
```

| Tipo | Donde vive | Valor inicial | Vive hasta |
|------|-----------|---------------|------------|
| Global | seccion de datos | **0** (garantizado) | fin del programa |
| `static` | seccion de datos | **0** (garantizado) | fin del programa |
| Local | **pila (stack)** | **BASURA** | fin de la funcion |
| `malloc` | **monticulo (heap)** | **BASURA** | hasta que hagas `free` |

> [!danger] El error mas comun de todos
> ```c
> int suma;           /* NO vale 0. Vale lo que hubiera en esa zona de pila */
> for (i=0; i<n; i++) suma += v[i];   /* resultado impredecible */
> ```
> **Siempre inicializa**: `int suma = 0;`
> Y ojo: muchas veces *parece* funcionar porque la basura era 0 por casualidad. Esos son los peores bugs.

### Ambito (scope)

```c
int x = 1;              /* global */
void f(void) {
    int x = 2;          /* local: OCULTA a la global dentro de f */
    printf("%d", x);    /* 2 */
}
```

---

## 5. Operadores

```c
+  -  *  /  %          /* aritmeticos */
==  !=  <  >  <=  >=   /* relacionales */
&&  ||  !              /* logicos (sobre condiciones) */
&   |   ^   ~  <<  >>  /* de BITS (sobre los bits del valor) */
++  --                 /* incremento/decremento */
+=  -=  *=  /=  %=     /* asignacion compuesta */
```

> [!warning] `=` vs `==`
> ```c
> if (x = 5) { ... }    /* ASIGNA 5 a x, y la condicion es 5 -> siempre CIERTA */
> if (x == 5) { ... }   /* compara. Esto es lo que querias */
> ```
> Compila sin error. `-Wall` te avisa con *"suggest parentheses around assignment"*.

> [!warning] Division entera
> `7 / 2` da **3**, no 3.5, porque ambos operandos son `int`.
> Para que salga 3.5: `7.0 / 2` o `(float)7 / 2`.

### `&&` / `||` vs `&` / `|`

| Operador | Trabaja sobre | `5 && 3` | `5 & 3` |
|----------|---------------|----------|---------|
| `&&` `\|\|` | verdadero/falso | `1` (ambos != 0) | |
| `&` `\|` | cada bit | | `1` (0101 & 0011 = 0001) |

En C **no existe el tipo booleano clasico**: `0` es falso, **cualquier otra cosa** es verdadero.

---

## 6. Control de flujo

```c
if (cond) { ... } else if (cond2) { ... } else { ... }

while (cond) { ... }

do { ... } while (cond);

for (i = 0; i < n; i++) { ... }

switch (c) {
    case 'a':
        ...
        break;          /* SIN break, cae al siguiente case */
    case 'b':
        ...
        break;
    default:
        ...
}
```

El `switch` es justo lo que vas a usar para procesar las opciones con `getopt()` en el ejercicio 3 de la Practica 1.

---

## Ejercicios

1. Escribe un programa que imprima el `sizeof` de todos los tipos y comparalo con la tabla.
2. Declara `char c = 127;`, incrementalo y explica el resultado.
3. Imprime el codigo ASCII de tu inicial usando `%d` y `%c` sobre la misma variable.
4. Declara `int suma;` sin inicializar, sumale 10 en un bucle e imprime. Ejecutalo varias veces.
5. Comprueba que `7/2` da 3 y consigue que de 3.5 de dos formas distintas.
6. Escribe `if (x = 5)` a proposito y mira que te dice `gcc -Wall`.
