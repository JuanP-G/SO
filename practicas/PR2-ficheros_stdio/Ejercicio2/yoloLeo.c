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

#define EXIT_USAGE       1
#define EXIT_FILE_ERROR  2

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
    size_t len = 0;
    char *str;

    /* ---- Fase 1: contar hasta encontrar el '\0' ---- */
    for (;;) {
        if (fread(&c, sizeof(c), 1, input) != 1) {
            /* No se pudo leer ni un byte más: fin de fichero o error. */
            if (ferror(input))
                warn("Error reading file");
            else if (len > 0)
                warnx("Truncated file: last string has no terminating '\\0'");

            return NULL;
        }

        if (c == '\0')
            break;

        len++;
    }

    /* ---- Fase 2: volver al principio de la cadena ---- */
    if (fseek(input, -(long)(len + 1), SEEK_CUR) != 0) {
        warn("fseek failed");
        return NULL;
    }

    /* ---- Fase 3: reservar y leer ---- */
    str = malloc(len + 1);
    if (str == NULL) {
        warn("malloc failed");
        return NULL;
    }

    if (fread(str, sizeof(*str), len + 1, input) != len + 1) {
        warn("Error reading string");
        free(str);
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
        exit(EXIT_USAGE);
    }

    if ((in = fopen(argv[1], "r")) == NULL)
        err(EXIT_FILE_ERROR, "Could not open %s", argv[1]);

    /* Pedimos cadenas hasta que loadstr devuelva NULL. */
    while ((s = loadstr(in)) != NULL) {
        puts(s);
        free(s);
    }

    fclose(in);
    return 0;
}
