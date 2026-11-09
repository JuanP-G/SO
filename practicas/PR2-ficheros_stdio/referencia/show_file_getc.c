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
