# SO - P2 - Corrección comentada (ficheros con stdio)

Volver a [Inicio](../../README.md) · Relacionado: [C 03 - Arrays y strings](../../fundamentos/c/03-arrays-y-strings.md) · [C 05 - Memoria dinamica](../../fundamentos/c/05-memoria-dinamica.md) · [C 06 - Funciones y structs](../../fundamentos/c/06-funciones-y-structs.md) · [Herramientas gcc make gdb](../../fundamentos/herramientas-gcc-make-gdb.md)

> Todo el código está compilado con `gcc -Wall -Wextra -g` **sin avisos** y probado (incluido valgrind limpio). Los comentarios explican el **porqué** de cada decisión. Los ficheros `.c` y el `Makefile` están en la carpeta `P2/` junto a esta nota.

## 0. Cómo compilar y usar

```bash
cd P2
make            # compila todo (regla genérica %: %.c)
make clean
```

## 1. Ideas de stdio que se usan en toda la práctica

| Función | Qué devuelve | Por qué hay que mirarlo |
|---|---|---|
| `fopen(f,"r"/"w"/"rb"/"wb")` | `FILE*` o `NULL` | `NULL` = no existe / sin permisos → `err(...)` |
| `fread/fwrite(ptr,size,n,f)` | nº de **elementos** (no bytes) | `0` o menos de lo pedido = EOF **o** error → `ferror(f)` |
| `getc/putc` | `int` (carácter o `EOF`) | por eso `int c`, no `char`: `EOF` es -1 y no cabe en un byte sin ambigüedad |
| `fclose` | 0 / `EOF` | puede fallar al vaciar el búfer (disco lleno) → comprobar al **escribir** |
| `fprintf/fscanf` | nº de campos/caracteres | `fscanf==3` = línea completa; `EOF` = fin |
| `fseek(f,off,SEEK_CUR)` | 0 / -1 | offset **negativo** retrocede; castear `(long)` antes de negar un `size_t` |
| `err(code,"msg")` / `errx` | no vuelve | `err` añade `strerror(errno)`; `errx` no (para errores que no son del sistema). Necesita `#include <err.h>` |

## 2. Ejercicio 1 – `show_file`

Versión con `fread`/`fwrite` (la pedida) y versión con `getc`/`putc` (la del enunciado, corregida).

### 2.1 `show_file.c`

```c
/*
 * show_file.c -- Ejercicio 1: mostrar un fichero por pantalla, byte a byte.
 *
 * VERSIÓN FINAL: usa fread()/fwrite() en lugar de getc()/putc().
 * (La versión original con getc/putc está en show_file_getc.c para comparar.)
 *
 * Uso: ./show_file <fichero>
 */

#include <stdio.h>   /* FILE, fopen, fread, fwrite, fclose, fprintf, stdout, stderr */
#include <stdlib.h>  /* exit() */
#include <err.h>     /* err(): imprime "programa: mensaje: <motivo del error>" y termina.
                        OJO: el enunciado NO incluye esta cabecera; sin ella gcc avisa
                        "implicit declaration of function 'err'". */

int main(int argc, char *argv[])
{
    FILE *file = NULL;       /* Inicializado a NULL: nunca dejamos un puntero sin valor. */

    /* ¿Por qué 'unsigned char' y no 'int'?
     *   - getc() devuelve un int porque necesita un valor EXTRA (EOF = -1) para decir
     *     "se acabó", y ese valor no puede confundirse con ningún byte (0..255).
     *   - fread() NO usa un valor centinela: avisa del final con su valor de RETORNO
     *     (0 elementos leídos). Así que aquí basta una variable de 1 byte, sin signo
     *     para que el byte 0xFF valga 255 y no -1. */
    unsigned char byte;

    /* 1. Comprobar los argumentos. Necesitamos exactamente: programa + fichero = 2. */
    if (argc != 2) {
        /* stderr y no stdout: los mensajes de error no deben mezclarse con los datos
         * que el programa escribe en stdout (si redirigimos con '>' los errores
         * seguirían viéndose en pantalla). */
        fprintf(stderr, "Usage: %s <file_name>\n", argv[0]);
        exit(1);             /* distinto de 0 = fallo (convenio del shell: $?) */
    }

    /* 2. Abrir el fichero. fopen() devuelve NULL si falla (no existe, sin permisos...).
     *    "rb" = lectura, modo binario. En Linux "r" y "rb" son idénticos, pero "rb"
     *    deja claro que copiamos bytes tal cual, sin traducir saltos de línea
     *    (en Windows el modo texto sí los traduce). */
    if ((file = fopen(argv[1], "rb")) == NULL)
        /* err() añade solo el motivo real ("No such file or directory"...) leyendo
         * la variable global errno. Con fprintf tendríamos que hacerlo a mano. */
        err(2, "The input file %s could not be opened", argv[1]);

    /* 3. Copiar byte a byte.
     *
     * fread(ptr, size, nmemb, stream):
     *   - ptr    : DÓNDE guardar lo leído. Por eso &byte (la dirección), igual que en scanf.
     *   - size   : tamaño de cada elemento (1 byte).
     *   - nmemb  : cuántos elementos queremos (1).
     *   - retorna: nº de ELEMENTOS leídos completos (no bytes). Aquí 1 = éxito;
     *              0 = fin de fichero O error (hay que distinguirlos después).
     */
    while (fread(&byte, 1, 1, file) == 1) {

        /* fwrite() es el espejo de fread(): escribe nmemb elementos de tamaño size
         * que están en ptr. Si devuelve menos de los pedidos, hubo un error
         * (disco lleno, tubería cerrada...). */
        if (fwrite(&byte, 1, 1, stdout) != 1) {
            fclose(file);                   /* liberar el recurso antes de salir */
            err(3, "fwrite() failed!!");
        }
    }

    /* fread() devolvió 0: ¿fin de fichero normal o error de lectura?
     * ferror() lo dice. Sin esta comprobación, un error de lectura a mitad de
     * fichero pasaría por "terminé bien" y no nos enteraríamos. */
    if (ferror(file)) {
        fclose(file);
        err(4, "Error reading %s", argv[1]);
    }

    fclose(file);   /* cerrar siempre lo que se abre: libera el descriptor y vacía buffers */
    return 0;       /* éxito. Al salir de main, stdout se vacía (flush) automáticamente. */
}
```

### 2.2 `show_file_getc.c`

```c
/*
 * show_file_getc.c -- Ejercicio 1: versión ORIGINAL del enunciado (getc/putc).
 *
 * Es el código del PDF tal cual, con UN solo cambio: se añade #include <err.h>
 * (sin él, gcc avisa de que 'err' no está declarada). Sirve para comparar su
 * salida con la de show_file.c: deben ser idénticas byte a byte.
 */

#include <stdio.h>
#include <stdlib.h>
#include <err.h>   /* <-- añadido: falta en el enunciado */

int main(int argc, char *argv[])
{
    FILE *file = NULL;
    int c, ret;   /* 'int' (no 'char'): getc devuelve un int para poder representar EOF (-1)
                     sin confundirlo con ningún byte válido 0..255 */

    if (argc != 2) {
        fprintf(stderr, "Usage: %s <file_name>\n", argv[0]);
        exit(1);
    }

    /* Open file */
    if ((file = fopen(argv[1], "r")) == NULL)
        err(2, "The input file %s could not be opened", argv[1]);

    /* Read file byte by byte */
    while ((c = getc(file)) != EOF) {
        /* Print byte to stdout. El cast a unsigned char evita que un byte >= 128
         * se convierta en un int negativo al pasarlo a putc. */
        ret = putc((unsigned char) c, stdout);

        if (ret == EOF) {
            fclose(file);
            err(3, "putc() failed!!");
        }
    }

    fclose(file);
    return 0;
}
```

**Qué cambia respecto al código del PDF:** falta `#include <err.h>` (sin él `err()` da *implicit declaration*), y `c` debe ser `int`. Ambas versiones dan salida **idéntica byte a byte** (probado con un fichero de 1024 bytes con todos los valores 0–255).

## 3. Ejercicio 2 – cadenas en fichero

Formato: cada cadena se guarda **con su `'\0'`**; ese byte es el delimitador. Por eso se escribe `strlen+1` bytes y `fputs/fprintf` **no** valen (no escriben el `'\0'`).

### 3.1 `write_strings.c`

```c
/*
 * write_strings.c -- Ejercicio 2 (1/2): escribir cadenas en un fichero.
 *
 * Guarda cada argumento como una cadena terminada en '\0'. El '\0' se escribe
 * también en el fichero: es lo que separa unas cadenas de otras (no hay otro
 * separador ni se guarda la longitud).
 *
 * Uso: ./write_strings <fichero> <cadena1> [<cadena2> ...]
 * Ej.: ./write_strings out London Paris Madrid
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>   /* strlen() */
#include <err.h>

int main(int argc, char *argv[])
{
    FILE *out;
    int i;

    /* Necesitamos: programa (1) + fichero (1) + al menos una cadena (1) = 3. */
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <file_name> <string1> [<string2> ...]\n", argv[0]);
        exit(1);
    }

    /* "w": crea el fichero si no existe y, si existe, LO VACÍA (lo que pide el
     * enunciado: "reescribirá su contenido"). Si quisiéramos añadir al final
     * habría que usar "a". */
    if ((out = fopen(argv[1], "w")) == NULL)
        err(2, "Could not open %s for writing", argv[1]);

    /* Las cadenas empiezan en argv[2]: argv[0] es el programa y argv[1] el fichero. */
    for (i = 2; i < argc; i++) {

        /* strlen() NO cuenta el '\0' final, pero nosotros SÍ queremos escribirlo
         * (es el delimitador que read_strings usará para saber dónde acaba cada
         * cadena). Por eso el +1. size_t es el tipo de strlen y de fwrite. */
        size_t len = strlen(argv[i]) + 1;

        /* fwrite(ptr, size, nmemb, stream): escribe nmemb elementos de 'size' bytes.
         * Con size = 1 y nmemb = len, escribe len bytes y devuelve cuántos escribió.
         * Si no coincide con len, hubo un error.
         *
         * ¿Por qué no fputs()/fprintf()? Porque NO escriben el '\0'. */
        if (fwrite(argv[i], 1, len, out) != len) {
            fclose(out);
            err(3, "Error writing to %s", argv[1]);
        }
    }

    /* fclose() vacía el buffer de stdio hacia el disco. Si el disco está lleno,
     * el error puede aparecer SOLO aquí, así que se comprueba su retorno. */
    if (fclose(out) != 0)
        err(4, "Error closing %s", argv[1]);

    return 0;
}
```

Comprobación: `./write_strings out London Paris Madrid Barcelona Berlin Lisbon` y `xxd out` coincide **exactamente** con el del enunciado.

### 3.2 `read_strings.c` (`loadstr`)

```c
/*
 * read_strings.c -- Ejercicio 2 (2/2): leer cadenas guardadas por write_strings.
 *
 * Lee del fichero cadenas terminadas en '\0' y las imprime, una por línea.
 *
 * Uso: ./read_strings <fichero>
 */

#include <stdio.h>
#include <stdlib.h>   /* malloc, free, exit */
#include <err.h>      /* err, warnx */

/*
 * loadstr -- lee UNA cadena terminada en '\0' desde la posición actual de 'input'.
 *
 * Devuelve: puntero a una cadena reservada con malloc (el que llama debe hacer
 *           free), o NULL si ya no quedan cadenas (fin de fichero) o hay un error.
 *
 * Idea del enunciado, en tres fases (no sabemos la longitud de antemano):
 *   1. Contar cuántos caracteres tiene la cadena, leyendo uno a uno.
 *   2. Retroceder ese mismo número de bytes con fseek().
 *   3. Reservar la memoria justa y leer la cadena completa de golpe.
 */
char *loadstr(FILE *input)
{
    char c;
    size_t len = 0;   /* nº de caracteres SIN contar el '\0' */
    char *str;

    /* ---- Fase 1: contar hasta encontrar el '\0' ---- */
    for (;;) {
        if (fread(&c, 1, 1, input) != 1) {
            /* No se pudo leer ni un byte más: fin de fichero o error. */
            if (ferror(input))
                warn("Error reading file");
            else if (len > 0)
                /* Habíamos leído letras pero se acabó el fichero sin '\0':
                 * el fichero está truncado o no es de este formato. */
                warnx("Truncated file: last string has no terminating '\\0'");
            /* Si len == 0 y no hay error es el final NORMAL: no quedan cadenas. */
            return NULL;
        }
        if (c == '\0')
            break;        /* fin de esta cadena (puede ser la cadena vacía, len == 0) */
        len++;
    }

    /* ---- Fase 2: volver al principio de la cadena ----
     * Hemos consumido len caracteres + el '\0' = len + 1 bytes. Retrocedemos
     * exactamente eso con un desplazamiento NEGATIVO respecto a la posición actual
     * (SEEK_CUR). El cast a long es imprescindible: len es size_t (sin signo) y
     * negar un entero sin signo da un número enorme, no un valor negativo. */
    if (fseek(input, -(long)(len + 1), SEEK_CUR) != 0) {
        warn("fseek failed");
        return NULL;
    }

    /* ---- Fase 3: reservar y leer ----
     * len + 1 bytes: los caracteres MÁS el '\0'. Como el '\0' viene del propio
     * fichero (write_strings lo escribió), al leer len + 1 bytes la cadena queda
     * ya terminada: no hace falta añadirlo a mano. */
    str = malloc(len + 1);
    if (str == NULL) {            /* malloc devuelve NULL si no hay memoria */
        warn("malloc failed");
        return NULL;
    }

    if (fread(str, 1, len + 1, input) != len + 1) {
        warn("Error reading string");
        free(str);                /* no filtrar memoria en el camino de error */
        return NULL;
    }

    return str;
}

int main(int argc, char *argv[])
{
    FILE *in;
    char *s;

    if (argc != 2) {
        fprintf(stderr, "Usage: %s <file_name>\n", argv[0]);
        exit(1);
    }

    if ((in = fopen(argv[1], "r")) == NULL)
        err(2, "Could not open %s", argv[1]);

    /* Pedimos cadenas hasta que loadstr devuelva NULL. La asignación dentro de la
     * condición guarda el resultado en 's' y a la vez lo compara con NULL. */
    while ((s = loadstr(in)) != NULL) {
        puts(s);    /* puts imprime la cadena + '\n': justo la separación pedida */
        free(s);    /* cada loadstr hizo un malloc => un free por cadena.
                       Sin esto, fuga de memoria (memory leak). */
    }

    fclose(in);
    return 0;
}
```

**Por qué en tres fases:** no sabemos cuánto mide la cadena hasta encontrar el `'\0'`. Fase 1 cuenta, fase 2 retrocede con `fseek`, fase 3 reserva exactamente `len+1` y lee de golpe. Una cadena vacía (solo `'\0'`) devuelve `""`, no `NULL`: así se distingue de EOF. Alternativa real: `getdelim(&buf,&n,'\0',f)`.

## 4. Ejercicio 3 – registros

`SimpleRecord`: `int id; double value; char label[16];` → `sizeof = 32` (4 de id + **4 de padding** + 8 de double + 16 de label).

### 4.1 A – formato texto

```c
/*
 * write_records_text.c  --  Ejercicio 3, parte A.1
 *
 * Escribe un array de registros en un fichero en FORMATO TEXTO:
 *     id valor etiqueta\n      (una línea por registro)
 *
 * Uso: ./write_records_text fichero_salida
 */
#include <stdio.h>
#include <stdlib.h>
#include <err.h>

#define LABEL_MAX_LEN 15   /* 15 caracteres útiles... */

typedef struct {
    int    id;
    double value;
    char   label[LABEL_MAX_LEN + 1];  /* ...+1 para el '\0' final: si no, un
                                         texto de 15 letras no cabría con su
                                         terminador y los str* se saldrían. */
} SimpleRecord;

/* 'static const': los datos no cambian y viven en memoria de solo lectura.
 * Los literales "Barcelona"... se copian dentro de label[] y el resto del
 * array se rellena con '\0' (así el contenido es determinista).
 * Los valores (3.10, 19.77, 7.42) son de ejemplo: ajústalos a los del PDF. */
static const SimpleRecord records[] = {
    {1, 3.10,  "Barcelona"},
    {0, 19.77, "Madrid"},
    {2, 7.42,  "Valencia"},
};

/* sizeof(array)/sizeof(elemento) = nº de elementos. Se calcula en compilación:
 * si añades un registro arriba, no hay que tocar nada más. */
#define NUM_RECORDS (sizeof(records) / sizeof(records[0]))

/* Escribe UN registro. Devuelve 0 si ok, -1 si error.
 * Separamos esta función para que conversion.c pueda reutilizar la idea. */
static int write_record_text(FILE *out, const SimpleRecord *r)
{
    /* %.2f: 2 decimales, suficiente para estos datos (se pierde precisión
     * si el double tuviera más: es una limitación del formato texto).
     * fprintf devuelve nº de caracteres escritos, negativo si falla. */
    if (fprintf(out, "%d %.2f %s\n", r->id, r->value, r->label) < 0)
        return -1;
    return 0;
}

int main(int argc, char *argv[])
{
    if (argc != 2) {
        fprintf(stderr, "Uso: %s fichero_salida\n", argv[0]);
        exit(1);
    }

    FILE *out = fopen(argv[1], "w");     /* "w": texto, crea/trunca */
    if (out == NULL)
        err(2, "No se puede abrir '%s'", argv[1]);   /* err añade strerror(errno) */

    for (size_t i = 0; i < NUM_RECORDS; i++) {
        if (write_record_text(out, &records[i]) != 0) {
            fclose(out);
            err(3, "Error escribiendo el registro %zu", i);
        }
    }

    /* fclose puede fallar (disco lleno): los datos pueden estar aún en el
     * búfer y el error solo aparece al vaciarlo. Por eso se comprueba. */
    if (fclose(out) != 0)
        err(4, "Error al cerrar '%s'", argv[1]);

    printf("La escritura ha finalizado correctamente\n");
    return 0;
}
```

```c
/*
 * read_records_text.c  --  Ejercicio 3, parte A.2
 *
 * Lee un fichero de texto "id valor etiqueta" y lo imprime por pantalla.
 * Uso: ./read_records_text fichero_entrada
 */
#include <stdio.h>
#include <stdlib.h>
#include <err.h>

#define LABEL_MAX_LEN 15

typedef struct {
    int    id;
    double value;
    char   label[LABEL_MAX_LEN + 1];
} SimpleRecord;

/* Devuelve: 1 = registro leído, 0 = fin de fichero limpio, -1 = error/formato. */
static int read_record_text(FILE *in, SimpleRecord *r)
{
    /* fscanf devuelve el nº de campos asignados (o EOF).
     *  - %lf (no %f): scanf necesita 'l' para double; con %f escribiría un
     *    float de 4 bytes sobre un double de 8 -> basura.
     *  - %15s: límite de ancho = LABEL_MAX_LEN. Sin él, una etiqueta larga
     *    desbordaría label[] (buffer overflow).
     *  - Los espacios del formato absorben cualquier blanco (incluido '\n'),
     *    por eso no hace falta poner \n al final. */
    int n = fscanf(in, "%d %lf %15s", &r->id, &r->value, r->label);
    if (n == 3)
        return 1;
    if (n == EOF)          /* nada más que leer */
        return ferror(in) ? -1 : 0;
    return -1;             /* 1 o 2 campos: línea mal formada */
}

int main(int argc, char *argv[])
{
    if (argc != 2) {
        fprintf(stderr, "Uso: %s fichero_entrada\n", argv[0]);
        exit(1);
    }

    FILE *in = fopen(argv[1], "r");
    if (in == NULL)
        err(2, "No se puede abrir '%s'", argv[1]);

    SimpleRecord r;
    int ret;
    while ((ret = read_record_text(in, &r)) == 1)
        printf("ID:%d, Valor:%.2f, Etiqueta: '%s'\n", r.id, r.value, r.label);

    /* El bucle acaba por EOF normal (ret==0) o por problema (ret==-1).
     * Hay que distinguirlos: confundirlos oculta ficheros corruptos. */
    if (ret < 0) {
        if (ferror(in)) {
            fclose(in);
            err(3, "Error de lectura en '%s'", argv[1]);
        }
        fclose(in);
        errx(4, "Formato incorrecto en '%s'", argv[1]);  /* errx: sin errno, no hay error de sistema */
    }

    fclose(in);   /* solo lectura: un fallo aquí no pierde datos */
    return 0;
}
```

Claves: `%lf` en `fscanf` (con `%f` corrompe el double), `%15s` (evita desbordar `label[16]`), distinguir EOF de formato incorrecto. **Limitación:** una etiqueta con espacios (`"San Sebastian"`) rompería el formato porque `%s` para en el espacio.

### 4.2 B – formato binario

```c
/*
 * write_records_bin.c  --  Ejercicio 3, parte B.1
 *
 * Escribe el array de registros en FORMATO BINARIO: copia byte a byte la
 * imagen en memoria de cada struct. Uso: ./write_records_bin fichero_salida
 */
#include <stdio.h>
#include <stdlib.h>
#include <err.h>

#define LABEL_MAX_LEN 15

typedef struct {
    int    id;
    double value;
    char   label[LABEL_MAX_LEN + 1];
} SimpleRecord;

/* Con 'static' (y const) el compilador inicializa todo el objeto a cero
 * antes de poner los valores, incluidos los bytes de relleno (padding) y los
 * restos de label[] tras el '\0'. Así el fichero es reproducible y no filtra
 * basura de memoria. */
static const SimpleRecord records[] = {
    {1, 3.10,  "Barcelona"},
    {0, 19.77, "Madrid"},
    {2, 7.42,  "Valencia"},
};
#define NUM_RECORDS (sizeof(records) / sizeof(records[0]))

int main(int argc, char *argv[])
{
    if (argc != 2) {
        fprintf(stderr, "Uso: %s fichero_salida\n", argv[0]);
        exit(1);
    }

    /* "wb": en Linux "w" y "wb" son iguales, pero "b" documenta la intención
     * y es imprescindible en Windows (si no, traduce \n por \r\n y corrompe
     * el binario). El enunciado dice "w": es una errata/descuido. */
    FILE *out = fopen(argv[1], "wb");
    if (out == NULL)
        err(2, "No se puede abrir '%s'", argv[1]);

    for (size_t i = 0; i < NUM_RECORDS; i++) {
        /* fwrite(ptr, tamaño_elemento, nº_elementos, stream) devuelve el nº
         * de ELEMENTOS escritos (no bytes). Esperamos 1. */
        if (fwrite(&records[i], sizeof(SimpleRecord), 1, out) != 1) {
            fclose(out);
            err(3, "Error escribiendo el registro %zu", i);
        }
    }
    /* Alternativa equivalente en una sola llamada:
     *   fwrite(records, sizeof(SimpleRecord), NUM_RECORDS, out) != NUM_RECORDS
     * Sirve aquí porque el array es contiguo y sin punteros (ver c5_label_dinamico.c). */

    if (fclose(out) != 0)
        err(4, "Error al cerrar '%s'", argv[1]);

    printf("La escritura ha finalizado correctamente\n");
    return 0;
}
```

```c
/*
 * read_records_bin.c  --  Ejercicio 3, parte B.2
 * Lee registros binarios y los imprime. Uso: ./read_records_bin fichero
 */
#include <stdio.h>
#include <stdlib.h>
#include <err.h>

#define LABEL_MAX_LEN 15

typedef struct {
    int    id;
    double value;
    char   label[LABEL_MAX_LEN + 1];
} SimpleRecord;

int main(int argc, char *argv[])
{
    if (argc != 2) {
        fprintf(stderr, "Uso: %s fichero_entrada\n", argv[0]);
        exit(1);
    }

    FILE *in = fopen(argv[1], "rb");
    if (in == NULL)
        err(2, "No se puede abrir '%s'", argv[1]);

    SimpleRecord r;
    /* fread devuelve nº de ELEMENTOS completos leídos. Con tamaño sizeof(r)
     * y cantidad 1, solo hay dos resultados: 1 (ok) o 0 (EOF/error/registro
     * truncado). Si hubiéramos pedido (1, sizeof(r)) un registro truncado
     * devolvería bytes sueltos y sería más difícil detectarlo. */
    while (fread(&r, sizeof(SimpleRecord), 1, in) == 1) {
        /* Defensa: si el fichero no es nuestro, label podría no acabar en
         * '\0' y printf("%s") leería fuera del struct. */
        r.label[LABEL_MAX_LEN] = '\0';
        printf("ID:%d, Valor:%.2f, Etiqueta: '%s'\n", r.id, r.value, r.label);
    }

    /* 0 puede ser EOF normal o error: se distingue con ferror/feof. */
    if (ferror(in)) {
        fclose(in);
        err(3, "Error de lectura en '%s'", argv[1]);
    }
    /* Resto de bytes que no completan un registro => fichero truncado.
     * feof sería verdadero igualmente, así que miramos si quedaron bytes. */
    long pos = ftell(in), size;
    fseek(in, 0, SEEK_END);
    size = ftell(in);
    if (pos >= 0 && size >= 0 && size % (long)sizeof(SimpleRecord) != 0)
        warnx("Aviso: '%s' tiene un tamaño (%ld) no múltiplo de %zu: posible truncado",
              argv[1], size, sizeof(SimpleRecord));

    fclose(in);
    return 0;
}
```

Errata del enunciado: dice abrir con `"w"`; para binario lo correcto es `"wb"` (en Linux da igual, en Windows `"w"` traduciría `\n`→`\r\n` y corrompería datos).

## 5. Parte C – preguntas (con datos medidos)

```
$ ls -l datos.txt datos.bin
-rw-r--r-- 1 u u 48 ... datos.txt
-rw-r--r-- 1 u u 96 ... datos.bin          (3 x 32)

$ cat -v datos.txt
1 3.10 Barcelona
0 19.77 Madrid
2 7.42 Valencia

$ xxd -c 32 datos.bin
00000000: 0100 0000 0000 0000 cdcc cccc cccc 0840 4261 7263 656c 6f6e 6100 0000 0000 0000
00000020: 0000 0000 0000 0000 85eb 51b8 1ec5 3340 4d61 6472 6964 0000 0000 0000 0000 0000
00000040: 0200 0000 0000 0000 ae47 e17a 14ae 1d40 5661 6c65 6e63 6961 0000 0000 0000 0000
```

`offsetof(value)=8`, `offsetof(label)=16`, `sizeof=32`.

| # | Pregunta | Respuesta |
|---|---|---|
| 1 | ¿Tamaños? | Texto **48 B** (depende de los datos: más dígitos/letras → más grande). Binario **96 B** = 3×32, **fijo** por registro. |
| 2 | ¿Qué se ve en el binario? | `01000000` = id=1 (little-endian), `00000000` = **padding**, `cdcccccccccc0840` = 3.10 en IEEE-754 (no legible), luego `"Barcelona"` y ceros hasta completar los 16 bytes. |
| 3 | ¿Por qué hay ceros de más? | Padding (alineación del `double` a múltiplo de 8) + resto de `label[]`. Por eso `records` es `static`: el compilador pone a 0 todo lo no inicializado. |
| 4 | ¿Portabilidad? | El binario depende de endianness, `sizeof(int)`, alineación/padding del compilador; el texto es portable (salvo separador decimal si hubiera `setlocale`). Binario: más compacto y rápido, acceso directo al registro *n* con `fseek(n*sizeof)`. Texto: legible/editable, depurable con `cat`. |
| 5 | ¿Y si `label` fuera `char *`? | Un único `fwrite(r,sizeof *r,…)` guardaría **la dirección** del puntero, no el texto → al leer en otra ejecución apunta a basura. Hay que serializar **campo a campo** (id, value, cadena + `'\0'`) y leer con `loadstr`, liberando cada `label` con `free`. Ver `c5_label_dinamico.c` (fichero de **62 B**, sin padding). |

### 5.1 Extra para C.5 – `c5_label_dinamico.c`

```c
/*
 * c5_label_dinamico.c -- Parte C.5: ¿y si label fuese 'char *'?
 *
 * Con char *label, el struct solo contiene la DIRECCIÓN de la cadena.
 * Un fwrite(r, sizeof(*r), ...) guardaría esa dirección, no el texto: al
 * leer en otra ejecución apunta a memoria inválida. Solución: serializar
 * campo a campo (id, value, texto + '\0').
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <err.h>

typedef struct { int id; double value; char *label; } DynRecord;

static void write_dyn(FILE *out, const DynRecord *r)
{
    size_t len = strlen(r->label) + 1;          /* +1: guardamos el '\0' como delimitador */
    if (fwrite(&r->id,    sizeof r->id,    1, out) != 1 ||
        fwrite(&r->value, sizeof r->value, 1, out) != 1 ||
        fwrite(r->label,  1, len, out) != len)
        err(3, "fwrite");
}

/* Igual que loadstr() de read_strings.c (versión corta: lee byte a byte
 * con realloc; en la práctica real se usaría getdelim()). */
static char *loadstr(FILE *in)
{
    size_t cap = 16, len = 0;
    char *s = malloc(cap);
    if (!s) err(1, "malloc");
    int c;
    while ((c = getc(in)) != EOF) {
        if (len + 1 >= cap) {
            cap *= 2;
            char *t = realloc(s, cap);
            if (!t) { free(s); err(1, "realloc"); }
            s = t;
        }
        s[len++] = (char)c;
        if (c == '\0') return s;
    }
    free(s);
    return NULL;                                 /* EOF antes del '\0' */
}

static int read_dyn(FILE *in, DynRecord *r)
{
    if (fread(&r->id, sizeof r->id, 1, in) != 1) return 0;
    if (fread(&r->value, sizeof r->value, 1, in) != 1) return -1;
    r->label = loadstr(in);
    return r->label ? 1 : -1;
}

int main(int argc, char *argv[])
{
    if (argc != 2) { fprintf(stderr, "Uso: %s fichero\n", argv[0]); return 1; }

    DynRecord recs[] = {
        {1, 3.10,  strdup("Barcelona")},
        {0, 19.77, strdup("Madrid")},
        {2, 7.42,  strdup("Valencia")},
    };
    size_t n = sizeof recs / sizeof recs[0];

    FILE *out = fopen(argv[1], "wb");
    if (!out) err(2, "fopen '%s'", argv[1]);
    for (size_t i = 0; i < n; i++) write_dyn(out, &recs[i]);
    if (fclose(out) != 0) err(4, "fclose");
    for (size_t i = 0; i < n; i++) free(recs[i].label);

    FILE *in = fopen(argv[1], "rb");
    if (!in) err(2, "fopen '%s'", argv[1]);
    DynRecord r;
    int ret;
    while ((ret = read_dyn(in, &r)) == 1) {
        printf("ID:%d, Valor:%.2f, Etiqueta: '%s'\n", r.id, r.value, r.label);
        free(r.label);                           /* cada label leída se reserva: hay que liberarla */
    }
    if (ret < 0) errx(5, "fichero corrupto");
    fclose(in);
    return 0;
}
```

## 6. Parte D (+ extra) – `conversion`

`./conversion [-i t|b] [-o t|b] entrada salida` — por defecto texto→texto; `-` = stdin/stdout.

```c
/*
 * conversion.c  --  Ejercicio 3, parte D (+ extra "-" = stdin/stdout)
 *
 * Convierte un fichero de registros entre formato texto (t) y binario (b).
 *   Uso: ./conversion [-i t|b] [-o t|b] entrada salida
 *   Por defecto: -i t -o t.   "-" como nombre = stdin / stdout.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>   /* getopt, optarg, optind */
#include <err.h>

#define LABEL_MAX_LEN 15

typedef struct {
    int    id;
    double value;
    char   label[LABEL_MAX_LEN + 1];
} SimpleRecord;

/* enum en vez de 'char' suelto: el compilador avisa de casos sin tratar en
 * un switch y el código se lee solo. */
typedef enum { FMT_TEXT, FMT_BIN } format_t;

struct options {
    format_t in_fmt, out_fmt;
    const char *in_name, *out_name;
};

static void usage(const char *prog)
{
    fprintf(stderr, "Uso: %s [-i t|b] [-o t|b] entrada salida\n", prog);
    exit(1);
}

/* strcmp, NO ==: comparar con == compararía direcciones de punteros, no el
 * contenido de las cadenas. */
static format_t parse_format(const char *s, const char *prog)
{
    if (strcmp(s, "t") == 0) return FMT_TEXT;
    if (strcmp(s, "b") == 0) return FMT_BIN;
    fprintf(stderr, "Formato desconocido '%s' (usa t o b)\n", s);
    usage(prog);
    return FMT_TEXT; /* inalcanzable: usage hace exit; evita warning */
}

static struct options parse_options(int argc, char *argv[])
{
    struct options o = { FMT_TEXT, FMT_TEXT, NULL, NULL };  /* valores por defecto */
    int c;

    /* "i:o:" -> las dos opciones llevan argumento (los ':'), que getopt
     * deja en la variable global optarg. Devuelve -1 al acabar las opciones. */
    while ((c = getopt(argc, argv, "i:o:")) != -1) {
        switch (c) {
        case 'i': o.in_fmt  = parse_format(optarg, argv[0]); break;
        case 'o': o.out_fmt = parse_format(optarg, argv[0]); break;
        default:  usage(argv[0]);   /* '?' : opción desconocida o sin argumento */
        }
    }

    /* getopt reordena argv: optind apunta al primer argumento que NO es
     * opción. Deben quedar exactamente dos (entrada y salida). */
    if (argc - optind != 2)
        usage(argv[0]);
    o.in_name  = argv[optind];
    o.out_name = argv[optind + 1];
    return o;
}

/* ---- lectura: devuelve 1 ok, 0 EOF limpio, -1 error ---- */
static int read_record(FILE *in, format_t fmt, SimpleRecord *r)
{
    if (fmt == FMT_TEXT) {
        int n = fscanf(in, "%d %lf %15s", &r->id, &r->value, r->label);
        if (n == 3) return 1;
        if (n == EOF) return ferror(in) ? -1 : 0;
        return -1;
    }
    if (fread(r, sizeof(*r), 1, in) == 1) {
        r->label[LABEL_MAX_LEN] = '\0';
        return 1;
    }
    return ferror(in) ? -1 : 0;   /* 0 bytes = EOF limpio */
}

/* ---- escritura: 0 ok, -1 error ---- */
static int write_record(FILE *out, format_t fmt, const SimpleRecord *r)
{
    if (fmt == FMT_TEXT)
        return fprintf(out, "%d %.2f %s\n", r->id, r->value, r->label) < 0 ? -1 : 0;
    return fwrite(r, sizeof(*r), 1, out) == 1 ? 0 : -1;
}

/* "-" significa entrada/salida estándar: permite usar el programa en
 * tuberías (pipes) de la shell. */
static FILE *open_file(const char *name, format_t fmt, int writing)
{
    if (strcmp(name, "-") == 0)
        return writing ? stdout : stdin;
    const char *mode = writing ? (fmt == FMT_BIN ? "wb" : "w")
                               : (fmt == FMT_BIN ? "rb" : "r");
    FILE *f = fopen(name, mode);
    if (f == NULL)
        err(2, "No se puede abrir '%s'", name);
    return f;
}

/* No se cierran stdin/stdout (los cierra el sistema al terminar), pero
 * stdout SÍ se vacía y se comprueba: es donde aparecería un error de
 * escritura (p. ej. pipe cerrado). */
static void close_file(FILE *f, const char *name)
{
    if (f == stdin) return;
    if (f == stdout) {
        if (fflush(stdout) != 0) err(4, "Error al escribir en la salida estándar");
        return;
    }
    if (fclose(f) != 0)
        err(4, "Error al cerrar '%s'", name);
}

int main(int argc, char *argv[])
{
    struct options o = parse_options(argc, argv);
    FILE *in  = open_file(o.in_name,  o.in_fmt,  0);
    FILE *out = open_file(o.out_name, o.out_fmt, 1);   /* ojo: si entrada==salida, "w" trunca antes de leer */

    SimpleRecord r;
    int ret;
    /* memset a cero ANTES de cada lectura: fscanf solo escribe hasta el '\0'
     * de la etiqueta y no toca ni los bytes sobrantes de label[] ni el
     * relleno (padding) del struct. Si luego hacemos fwrite del struct
     * entero, esos bytes sin inicializar acabarían en el fichero binario
     * (basura de la pila, no reproducible; valgrind lo detecta). */
    memset(&r, 0, sizeof r);
    while ((ret = read_record(in, o.in_fmt, &r)) == 1) {
        if (write_record(out, o.out_fmt, &r) != 0)
            err(3, "Error escribiendo en '%s'", o.out_name);
        memset(&r, 0, sizeof r);   /* limpiar para el siguiente registro */
    }

    if (ret < 0) {
        if (ferror(in)) err(3, "Error de lectura en '%s'", o.in_name);
        errx(5, "Formato de entrada incorrecto en '%s'", o.in_name);
    }

    close_file(out, o.out_name);
    close_file(in,  o.in_name);
    return 0;
}
```

**Hallazgo de la prueba con valgrind:** sin los `memset`, al convertir texto→binario el fichero contenía **basura de pila** tras el `'\0'` de la etiqueta y en el padding (`fscanf` solo escribe hasta el `'\0'`). Con `memset` el resultado es **idéntico byte a byte** al de `write_records_bin`.

### Comprobaciones en la shell

```bash
./conversion -o b datos.txt c.bin && cmp c.bin datos.bin && echo igual   # t->b
./conversion -i b datos.bin c.txt && diff c.txt datos.txt && echo igual   # b->t
cat datos.txt | ./conversion -o b - - | ./conversion -i b - -             # pipeline (ida y vuelta)
./conversion -o b datos.txt - | xxd | head                                # ver binario en pantalla
./conversion -i x datos.txt o      # formato desconocido -> usage, exit 1
```

> Cuidado: si entrada y salida son el **mismo fichero**, `fopen(...,"w")` lo trunca antes de leer y pierdes los datos.

## 7. Makefile

```make
# Makefile de la Práctica 2
# -Wall -Wextra: avisos útiles; -g: símbolos para gdb/valgrind.
CC     = gcc
CFLAGS = -Wall -Wextra -g

PROGS = show_file show_file_getc write_strings read_strings \
        write_records_text read_records_text \
        write_records_bin read_records_bin \
        conversion c5_label_dinamico

all: $(PROGS)

# Regla genérica: cada programa se construye desde su .c homónimo.
# $@ = destino (el ejecutable)   $< = primer prerrequisito (el .c)
%: %.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f $(PROGS) *.o

# all y clean no son ficheros: .PHONY evita confundirlos con uno que se llame así
.PHONY: all clean
```

## 8. Erratas / ambigüedades del enunciado

- Falta `#include <err.h>` en el código de `show_file`.
- El texto dice formato de línea "id valor etiqueta", pero el `cat` de ejemplo muestra "etiqueta valor id": sigue el orden que se te indique; yo uso **id valor etiqueta**.
- Ejemplo `./write_records_bin.c` → debería ser `./write_records_bin` (si no, "Permiso denegado", como te pasó en P1).
- B.1 pide `"w"` para binario → mejor `"wb"`.
- La descripción de `-o` dice "formato de entrada": es de **salida**.

## 9. Checklist de examen

- [ ] ¿Compruebo el retorno de `fopen`, `fread/fwrite` y `fclose` (en escritura)?
- [ ] ¿`fread/fwrite` cuentan **elementos**, no bytes?
- [ ] ¿Distingo EOF de error con `ferror`?
- [ ] ¿`int c` con `getc`?
- [ ] ¿`%lf` en `scanf` y ancho en `%s`?
- [ ] ¿Cadenas guardadas con `'\0'`? ¿`free` de lo que `malloc`é?
- [ ] ¿Strings con `strcmp`, nunca `==`?
- [ ] ¿Estructuras con punteros → serializar campo a campo?
- [ ] ¿`memset` antes de volcar un struct a binario?
