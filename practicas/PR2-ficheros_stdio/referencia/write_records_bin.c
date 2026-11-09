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
