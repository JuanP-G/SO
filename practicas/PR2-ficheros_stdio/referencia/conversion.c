/*
 * conversion.c  --  Ejercicio 3, parte D (+ extra "-" = stdin/stdout)
 *
 * Convierte un fichero de registros entre formato texto (t) y binario (b).
 *   Uso: ./conversion [-i t|b] [-o t|b] entrada salida
 *   Por defecto: -i t -o t.   "-" como nombre = stdin / stdout.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>   /* getopt, optarg, optind */
#include <err.h>

#define LABEL_MAX_LEN 15

typedef struct {
    int    id;
    double value;
    char   label[LABEL_MAX_LEN + 1];
} SimpleRecord;

/* enum en vez de 'char' suelto: el compilador avisa de casos sin tratar en
 * un switch y el código se lee solo. */
typedef enum { FMT_TEXT, FMT_BIN } format_t;

struct options {
    format_t in_fmt, out_fmt;
    const char *in_name, *out_name;
};

static void usage(const char *prog)
{
    fprintf(stderr, "Uso: %s [-i t|b] [-o t|b] entrada salida\n", prog);
    exit(1);
}

/* strcmp, NO ==: comparar con == compararía direcciones de punteros, no el
 * contenido de las cadenas. */
static format_t parse_format(const char *s, const char *prog)
{
    if (strcmp(s, "t") == 0) return FMT_TEXT;
    if (strcmp(s, "b") == 0) return FMT_BIN;
    fprintf(stderr, "Formato desconocido '%s' (usa t o b)\n", s);
    usage(prog);
    return FMT_TEXT; /* inalcanzable: usage hace exit; evita warning */
}

static struct options parse_options(int argc, char *argv[])
{
    struct options o = { FMT_TEXT, FMT_TEXT, NULL, NULL };  /* valores por defecto */
    int c;

    /* "i:o:" -> las dos opciones llevan argumento (los ':'), que getopt
     * deja en la variable global optarg. Devuelve -1 al acabar las opciones. */
    while ((c = getopt(argc, argv, "i:o:")) != -1) {
        switch (c) {
        case 'i': o.in_fmt  = parse_format(optarg, argv[0]); break;
        case 'o': o.out_fmt = parse_format(optarg, argv[0]); break;
        default:  usage(argv[0]);   /* '?' : opción desconocida o sin argumento */
        }
    }

    /* getopt reordena argv: optind apunta al primer argumento que NO es
     * opción. Deben quedar exactamente dos (entrada y salida). */
    if (argc - optind != 2)
        usage(argv[0]);
    o.in_name  = argv[optind];
    o.out_name = argv[optind + 1];
    return o;
}

/* ---- lectura: devuelve 1 ok, 0 EOF limpio, -1 error ---- */
static int read_record(FILE *in, format_t fmt, SimpleRecord *r)
{
    if (fmt == FMT_TEXT) {
        int n = fscanf(in, "%d %lf %15s", &r->id, &r->value, r->label);
        if (n == 3) return 1;
        if (n == EOF) return ferror(in) ? -1 : 0;
        return -1;
    }
    if (fread(r, sizeof(*r), 1, in) == 1) {
        r->label[LABEL_MAX_LEN] = '\0';
        return 1;
    }
    return ferror(in) ? -1 : 0;   /* 0 bytes = EOF limpio */
}

/* ---- escritura: 0 ok, -1 error ---- */
static int write_record(FILE *out, format_t fmt, const SimpleRecord *r)
{
    if (fmt == FMT_TEXT)
        return fprintf(out, "%d %.2f %s\n", r->id, r->value, r->label) < 0 ? -1 : 0;
    return fwrite(r, sizeof(*r), 1, out) == 1 ? 0 : -1;
}

/* "-" significa entrada/salida estándar: permite usar el programa en
 * tuberías (pipes) de la shell. */
static FILE *open_file(const char *name, format_t fmt, int writing)
{
    if (strcmp(name, "-") == 0)
        return writing ? stdout : stdin;
    const char *mode = writing ? (fmt == FMT_BIN ? "wb" : "w")
                               : (fmt == FMT_BIN ? "rb" : "r");
    FILE *f = fopen(name, mode);
    if (f == NULL)
        err(2, "No se puede abrir '%s'", name);
    return f;
}

/* No se cierran stdin/stdout (los cierra el sistema al terminar), pero
 * stdout SÍ se vacía y se comprueba: es donde aparecería un error de
 * escritura (p. ej. pipe cerrado). */
static void close_file(FILE *f, const char *name)
{
    if (f == stdin) return;
    if (f == stdout) {
        if (fflush(stdout) != 0) err(4, "Error al escribir en la salida estándar");
        return;
    }
    if (fclose(f) != 0)
        err(4, "Error al cerrar '%s'", name);
}

int main(int argc, char *argv[])
{
    struct options o = parse_options(argc, argv);
    FILE *in  = open_file(o.in_name,  o.in_fmt,  0);
    FILE *out = open_file(o.out_name, o.out_fmt, 1);   /* ojo: si entrada==salida, "w" trunca antes de leer */

    SimpleRecord r;
    int ret;
    /* memset a cero ANTES de cada lectura: fscanf solo escribe hasta el '\0'
     * de la etiqueta y no toca ni los bytes sobrantes de label[] ni el
     * relleno (padding) del struct. Si luego hacemos fwrite del struct
     * entero, esos bytes sin inicializar acabarían en el fichero binario
     * (basura de la pila, no reproducible; valgrind lo detecta). */
    memset(&r, 0, sizeof r);
    while ((ret = read_record(in, o.in_fmt, &r)) == 1) {
        if (write_record(out, o.out_fmt, &r) != 0)
            err(3, "Error escribiendo en '%s'", o.out_name);
        memset(&r, 0, sizeof r);   /* limpiar para el siguiente registro */
    }

    if (ret < 0) {
        if (ferror(in)) err(3, "Error de lectura en '%s'", o.in_name);
        errx(5, "Formato de entrada incorrecto en '%s'", o.in_name);
    }

    close_file(out, o.out_name);
    close_file(in,  o.in_name);
    return 0;
}
