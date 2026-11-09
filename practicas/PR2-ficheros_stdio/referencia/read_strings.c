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
