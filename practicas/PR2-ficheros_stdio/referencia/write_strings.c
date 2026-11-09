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
