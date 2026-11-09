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
