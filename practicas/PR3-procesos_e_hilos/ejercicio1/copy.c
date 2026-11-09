#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>     /* open, O_RDONLY, O_WRONLY, O_CREAT, O_TRUNC */
#include <unistd.h>    /* read, write, close */
#include <err.h>

#define BLOCK_SIZE 512

#define COPY_OK          0
#define COPY_READ_ERROR  1
#define COPY_WRITE_ERROR 2

/* Copia todo lo que se pueda leer de fd_src a fd_dst en bloques de 512 bytes. */
static int copy(int fd_src, int fd_dst)
{
	char buffer[BLOCK_SIZE];
	ssize_t bytes_read;

	while ((bytes_read = read(fd_src, buffer, sizeof(buffer))) > 0)
		if (write(fd_dst, buffer, bytes_read) != bytes_read)
			return COPY_WRITE_ERROR;

	if (bytes_read == -1)
		return COPY_READ_ERROR;

	return COPY_OK;
}

int main(int argc, char *argv[])
{
	//Primero compurebo que no falten argumentos en la llamada
	if (argc != 3) {
		fprintf(stderr, "Uso: %s origen destino\n", argv[0]);
		exit(1);
	}
	//Abrimos el fichero que nos dan
	int fd_in = open(argv[1], O_RDONLY);
	//Mientras no haya ningun error continuamos
	if (fd_in == -1)
		err(2, "No se puede abrir '%s'", argv[1]);

	//Ahora abrimos la salida si no existe aun la crea y si ya existe lo vacia
	int fd_out = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC, 0644);
	if (fd_out == -1) {
		close(fd_in);
		err(3, "No se puede abrir '%s'", argv[2]);
	}
	
	//Ahora llamamos a la copia con los ficheros de entrada y salida
	int result = copy(fd_in, fd_out);

	if (result == COPY_WRITE_ERROR) {
		close(fd_in);
		close(fd_out);
		err(4, "Error escribiendo en '%s'", argv[2]);
	}
    
	if (result == COPY_READ_ERROR) {
		close(fd_in);
		close(fd_out);
		err(5, "Error leyendo '%s'", argv[1]);
	}

	//Ahora cerramos el fd_in y compruebo que no haya fallado 
	close(fd_in);
	if (close(fd_out) == -1)
		err(6, "Error al cerrar '%s'", argv[2]);

	return 0;
}
