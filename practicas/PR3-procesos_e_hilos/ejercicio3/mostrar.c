#include <stdio.h>     /* fprintf */
#include <stdlib.h>    /* atol, exit */
#include <fcntl.h>     /* open */
#include <unistd.h>    /* getopt, optarg, optind, lseek, read, write, close */
#include <err.h>

int main(int argc, char *argv[])
{
	long n = 0;		/* bytes a saltar, o últimos N si hay -e */
	int from_end = 0;	/* 0: saltar N desde el inicio; 1: mostrar los últimos N */
	int opt;

	/* Opciones: -n lleva valor (optarg), -e es un interruptor */
	while ((opt = getopt(argc, argv, "n:e")) != -1) {
		switch (opt) {
		case 'n':
			n = atol(optarg);
			break;
		case 'e':
			from_end = 1;
			break;
		default:
			fprintf(stderr, "Uso: %s [-n N] [-e] fichero\n", argv[0]);
			exit(1);
		}
	}

	/* Tras las opciones debe quedar el nombre del fichero: argv[optind] */
	if (optind >= argc) {
		fprintf(stderr, "Uso: %s [-n N] [-e] fichero\n", argv[0]);
		exit(1);
	}

	int fd = open(argv[optind], O_RDONLY);
	if (fd == -1)
		err(2, "No se puede abrir '%s'", argv[optind]);

	/* Colocar el marcador antes de leer */
	if (from_end) {
		if (lseek(fd, -n, SEEK_END) == -1)	/* -n: retroceder desde el final */
			err(3, "No se puede situar el marcador");
	} else {
		if (lseek(fd, n, SEEK_SET) == -1)	/* avanzar n desde el principio */
			err(3, "No se puede situar el marcador");
	}

	/* Copiar byte a byte hasta EOF a la salida estándar (descriptor 1) */
	char byte;
	ssize_t bytes_read;
	while ((bytes_read = read(fd, &byte, sizeof(byte))) > 0) {
		if (write(STDOUT_FILENO, &byte, sizeof(byte)) != sizeof(byte)) {
			close(fd);
			err(4, "Error escribiendo");
		}
	}
	if (bytes_read == -1) {		/* el bucle salió por error, no por EOF */
		close(fd);
		err(5, "Error leyendo '%s'", argv[optind]);
	}

	close(fd);
	return 0;
}