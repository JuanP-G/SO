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
