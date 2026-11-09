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

