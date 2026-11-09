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
