#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>     /* open, O_RDONLY, O_WRONLY, O_CREAT, O_TRUNC */
#include <unistd.h>    /* read, write, close */
#include <err.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>


#define BLOCK_SIZE 512

#define COPY_OK				0
#define COPY_READ_ERROR		1
#define COPY_WRITE_ERROR	2


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

static void copy_regular(const char *src, const char *dst){
	//Abrimos el fichero que nos dan
	int fd_in = open(src, O_RDONLY);
	//Mientras no haya ningun error continuamos
	if (fd_in == -1)
		err(3, "No se puede abrir '%s'", src);

	//Ahora abrimos la salida si no existe aun la crea y si ya existe lo vacia
	int fd_out = open(dst, O_WRONLY | O_CREAT | O_TRUNC, 0644);
	if (fd_out == -1) {
		close(fd_in);
		err(4, "No se puede abrir '%s'", dst);
	}

	//Ahora llamamos a la copia con los ficheros de entrada y salida
	int result = copy(fd_in, fd_out);

	if (result == COPY_WRITE_ERROR) {
		close(fd_in);
		close(fd_out);
		err(5, "Error escribiendo en '%s'", dst);
	}
    
	if (result == COPY_READ_ERROR) {
		close(fd_in);
		close(fd_out);
		err(6, "Error leyendo '%s'", src);
	}

	//Ahora cerramos el fd_in y compruebo que no haya fallado 
	close(fd_in);
	if (close(fd_out) == -1)
		err(7, "Error al cerrar '%s'", dst);
}

static void copy_symlink(const char *src, const char *dst, off_t link_size)
{
	/* st_size no cuenta el '\0', así que reservamos uno más */
	char *target = malloc(link_size + 1);
	if (target == NULL)
		err(3, "malloc");

	ssize_t len = readlink(src, target, link_size);
	if (len == -1) {
		free(target);
		err(4, "No se puede leer el enlace '%s'", src);
	}
	target[len] = '\0';	/* readlink no lo añade */

	if (symlink(target, dst) == -1) {
		free(target);
		err(5, "No se puede crear el enlace '%s'", dst);
	}
	free(target);
}

int main(int argc, char *argv[])
{

	struct stat st;

	//Primero compurebo que no falten argumentos en la llamada
	if (argc != 3) {
		fprintf(stderr, "Uso: %s origen destino\n", argv[0]);
		exit(1);
	}
    
	/* Primero el tipo (lstat no sigue enlaces) y solo después tocamos el destino */
	if (lstat(argv[1], &st) == -1)
		err(2, "No se puede obtener información de '%s'", argv[1]);


	//Ahora con la respuesta de lstat comprobamos de que tipo es el archivo
	if (S_ISREG(st.st_mode))
		copy_regular(argv[1], argv[2]);
	else if (S_ISLNK(st.st_mode))
		copy_symlink(argv[1], argv[2], st.st_size);
	else //En caso de ser otro que no sea regular o unenlace simbolico lanzamos error
		errx(8, "'%s' no es un fichero regular ni un enlace simbólico", argv[1]);
   
	return 0;
}
