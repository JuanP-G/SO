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
