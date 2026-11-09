# SO - C 03 - Arrays y strings

Parte de [Inicio](../../README.md). Anterior: [C 02 - Tipos y variables](02-tipos-y-variables.md) | Siguiente: [C 04 - Punteros](04-punteros.md)

---

## 1. Arrays

```c
int v[5];                        /* 5 enteros SIN inicializar -> basura */
int w[5] = {1, 2, 3, 4, 5};      /* inicializado */
int z[5] = {0};                  /* TODO a cero (truco util) */
int a[]  = {1, 2, 3};            /* el tamano lo deduce: 3 */
```

En memoria son **posiciones contiguas**:

```
int v[5];   (int = 4 bytes)

  v[0]   v[1]   v[2]   v[3]   v[4]
┌──────┬──────┬──────┬──────┬──────┐
│      │      │      │      │      │
└──────┴──────┴──────┴──────┴──────┘
 1000   1004   1008   1012   1016
   ^
   └── v  (el nombre del array = direccion del primer elemento)
```

> [!danger] C NO comprueba los indices
> ```c
> int v[5];
> v[10] = 99;     /* compila. Escribe en memoria que no es tuya. */
> ```
> No hay `ArrayIndexOutOfBoundsException`. O corrompes otra variable, o revientas con *segmentation fault*, o **no pasa nada visible** y el bug aparece en otro sitio. Los indices van de `0` a `n-1`. Siempre.

### `sizeof` sobre un array

```c
int v[10];
sizeof(v)            /* 40  = 10 elementos * 4 bytes */
sizeof(v)/sizeof(v[0])   /* 10 = numero de ELEMENTOS  <-- el idioma estandar */
```

> [!important] Ejercicio 1.4: el array "se degrada" al pasarlo a una funcion
> ```c
> void f(int arr[]) {
>     printf("%lu\n", sizeof(arr));   /* 8, NO 40 */
> }
> ```
> Al pasar un array a una funcion **se pasa solo la direccion del primer elemento**, no una copia. Dentro de la funcion `arr` es un `int*`, y `sizeof(int*)` es 8 (el tamano de un puntero en 64 bits).
>
> **Por eso hay que pasar el tamano como segundo argumento.** La funcion no tiene forma de saberlo.
>
> Y si la funcion declara su propio array local (`init_array2`), ahi `sizeof` si funciona: es un array de verdad, no un puntero.

### Copiar arrays

```c
int a[5] = {1,2,3,4,5};
int b[5];
b = a;              /* ERROR de compilacion: no se puede asignar un array */

for (i=0; i<5; i++) b[i] = a[i];    /* asi si */
memcpy(b, a, sizeof(a));            /* o asi, con <string.h> */
```

Esa es la respuesta al `array2.c` del ejercicio 1.4.

---

## 2. Cadenas de caracteres

**En C no existe el tipo `String`.** Una cadena es un `char[]` que termina en el caracter nulo `'\0'` (valor 0).

```c
char s[6] = {'h','o','l','a','!','\0'};
char s[] = "hola!";        /* equivalente. El '\0' lo pone el compilador */
```

```
char s[] = "hola";

  s[0] s[1] s[2] s[3] s[4]
┌────┬────┬────┬────┬────┐
│ h  │ o  │ l  │ a  │ \0 │     <-- 5 bytes para 4 letras
└────┴────┴────┴────┴────┘
 104   111  108  97    0
```

> [!important] La regla de oro de las cadenas en C
> **Una cadena de N caracteres necesita N+1 bytes.** El `'\0'` es el que le dice a `printf`, `strlen`, `strcpy`... donde parar. Si falta, esas funciones siguen leyendo memoria hasta que encuentran un 0 por casualidad, o petan.

### `strlen()` vs `sizeof()`

```c
char s[20] = "hola";
strlen(s)     /* 4  -> cuenta caracteres HASTA el '\0', en EJECUCION */
sizeof(s)     /* 20 -> tamano del array reservado, en COMPILACION */

char *p = "hola";
strlen(p)     /* 4  */
sizeof(p)     /* 8  -> es un PUNTERO, mide el puntero */
```

| | `strlen()` | `sizeof()` |
|---|-----------|-----------|
| Que es | funcion de `<string.h>` | operador del lenguaje |
| Cuando | en ejecucion (recorre) | en compilacion |
| Mide | caracteres hasta `'\0'` | bytes que ocupa el tipo |
| Cuenta el `'\0'`? | no | si (si es un array) |

---

## 3. `char*` vs `char[]` - el tema estrella del ejercicio 1.3 y 1.7

```c
char *str1  = "STRING OF CHARACTERS";   /* PUNTERO a literal */
char  str2[] = "STRING OF CHARACTERS";  /* ARRAY con copia propia */
```

```
  str1 (puntero, 8 bytes)              zona .rodata (SOLO LECTURA)
 ┌──────────────┐                     ┌─────────────────────┐
 │  0x400604    │ ──────────────────> │ STRING OF CHARA...\0│
 └──────────────┘                     └─────────────────────┘

  str2 (array, 21 bytes, en la PILA, copia propia y modificable)
 ┌──┬──┬──┬──┬──┬──┬─ ... ─┬──┐
 │S │T │R │I │N │G │       │\0│
 └──┴──┴──┴──┴──┴──┴─ ... ─┴──┘
```

| | `char *str1 = "..."` | `char str2[] = "..."` |
|---|---------------------|----------------------|
| Que es | un puntero (8 bytes) que apunta a un literal | un array con su propia copia |
| `sizeof` | **8** (el puntero) | **21** (20 letras + `\0`) |
| Se puede modificar? | **NO** -> *segmentation fault* | SI |
| `str1 = otra;` | si, reapunta | no, no se puede asignar un array |
| Donde vive el texto | zona de solo lectura del ejecutable | en la pila, dentro de la funcion |

```c
str1[0] = 'X';    /* CRASH en ejecucion: segmentation fault */
str2[0] = 'X';    /* perfecto */
```

> [!danger] Ejercicio 1.7 (`strings1.c`): "el error se manifiesta en compilacion o en ejecucion?"
> **En ejecucion.** El compilador te deja escribir `str1[0]='X'` sin rechistar (como mucho un warning). El sistema operativo es quien mata el proceso cuando intenta escribir en una pagina de memoria marcada como solo lectura. Esto es gestion de memoria del SO en accion: Modulo 5.

### `p = msg2;` no copia nada

```c
char *p;
char msg2[] = "Bonjour";
p = msg2;          /* p apunta al MISMO sitio. NO hay copia. */
p[0] = 'b';        /* tambien cambia msg2[0] */
```

Copiar de verdad requiere `strcpy()` o recorrer el array a mano.

### Comparar cadenas

```c
char *s1 = "hola";
char s2[] = "hola";

if (s1 == s2)              /* MAL: compara DIRECCIONES. Siempre falso aqui */
if (strcmp(s1, s2) == 0)   /* BIEN: compara CONTENIDO */
```

`strcmp` devuelve **0 si son iguales**, negativo si `s1 < s2`, positivo si `s1 > s2`. Que 0 signifique "iguales" despista al principio.

---

## 4. Funciones de `<string.h>`

| Funcion | Que hace | Peligro |
|---------|----------|---------|
| `strlen(s)` | longitud sin el `\0` | s debe acabar en `\0` |
| `strcpy(dst, src)` | copia src en dst **incluido el `\0`** | **no comprueba el tamano de dst** |
| `strncpy(dst, src, n)` | copia como mucho n bytes | si src tiene >= n, **no pone el `\0`** |
| `strcat(dst, src)` | concatena src al final de dst | mismo desbordamiento |
| `strncat(dst, src, n)` | idem limitado | |
| `strcmp(a, b)` | compara. **0 = iguales** | |
| `strncmp(a, b, n)` | compara n primeros | |
| `strchr(s, c)` | puntero a la 1a aparicion de c, o NULL | |
| `strstr(s, sub)` | puntero a la 1a aparicion de sub, o NULL | |
| `strdup(s)` | **reserva memoria nueva y copia s ahi** | hay que hacer `free()` |
| `strsep(&s, delim)` | parte una cadena en tokens | **modifica la cadena original** |
| `sprintf(buf, fmt, ...)` | como printf pero a un buffer | desbordamiento |
| `snprintf(buf, n, ...)` | idem con limite | la version segura |

> [!danger] `strcpy` y el desbordamiento de buffer
> ```c
> char dst[5];
> strcpy(dst, "esto es larguisimo");   /* escribe 19 bytes en 5 */
> ```
> Compila. Ejecuta. Machaca lo que hubiera detras en la pila. Esta es la base historica de la mitad de los exploits de seguridad del mundo.

### `strdup()` - ejercicio 4 de la Practica 1

```c
char *copia = strdup(original);   /* malloc + strcpy en una sola llamada */
/* ... */
free(copia);                      /* hay que liberarlo */
```

Equivale a:

```c
char *copia = malloc(strlen(original) + 1);   /* +1 por el '\0' !! */
strcpy(copia, original);
```

Por eso `clone_string()` existe en `show-passwd.c`: **no basta con `campo = cadena_existente;`** porque eso solo apunta a memoria que va a ser reutilizada o destruida en la siguiente iteracion. Hay que hacer una copia propia.

### `strsep()` - trocear por un delimitador

```c
char line[] = "juan:x:1000:1000::/home/juan:/bin/bash";
char *resto = line;
char *token;
while ((token = strsep(&resto, ":")) != NULL) {
    printf("[%s]\n", token);
}
```

Como funciona: busca el primer `:`, **lo sustituye por `'\0'`**, devuelve el puntero al principio del token y avanza `resto` al caracter siguiente.

```
antes:   j u a n : x : 1 0 0 0 ...
                  ^ resto apunta a 'j'

tras 1a llamada:
         j u a n \0 x : 1 0 0 0 ...
         ^devuelto    ^resto
```

Por eso el enunciado avisa de que `strsep()` **modifica la cadena original**: la va rellenando de `'\0'`. Y por eso hay que pasarle un puntero a puntero (`char **`): necesita poder modificar tu variable `resto`.

---

## Ejercicios

1. Declara `char s[] = "Sistemas"` e imprime `strlen(s)` y `sizeof(s)`. Explica la diferencia.
2. Haz lo mismo con `char *p = "Sistemas"`.
3. Intenta modificar el primer caracter de las dos. Cual peta?
4. Escribe tu propia version de `strlen()` con un bucle `while`.
5. Recorre una cadena imprimiendo cada caracter y su codigo ASCII.
6. Copia una cadena a otra a mano, sin usar `strcpy`. Acuerdate del `'\0'`.
7. Usa `strsep` para trocear `"a:b:c"` e imprime cada token.
