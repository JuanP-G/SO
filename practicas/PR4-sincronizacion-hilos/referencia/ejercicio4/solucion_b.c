/* Cuestión B: el padre escribe su "00000" entre los bloques de los hijos.
 * Aquí SÍ se aprovecha que padre e hijos comparten el mismo descriptor, y por tanto el
 * mismo desplazamiento: cada escritura avanza la posición para todos. Para que el orden
 * sea el correcto, el padre espera a cada hijo antes de seguir. */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>

#define NUM_PROCESSES 10
#define BLOCK_SIZE    5

int main(void)
{
    int fd = open("output.txt", O_CREAT | O_TRUNC | O_RDWR, S_IRUSR | S_IWUSR);
    if (fd == -1) {
        perror("open");
        return 1;
    }

    for (int i = 1; i < NUM_PROCESSES; i++) {
        write(fd, "00000", BLOCK_SIZE);            /* el padre, antes de cada hijo */

        if (fork() == 0) {
            char text[BLOCK_SIZE + 1];
            snprintf(text, sizeof(text), "%05d", i * 11111);
            write(fd, text, BLOCK_SIZE);           /* sin lseek: continúa donde dejó el padre */
            exit(0);
        }
        wait(NULL);                                /* el hijo termina antes del siguiente bloque */
    }

    char c;
    lseek(fd, 0, SEEK_SET);
    printf("File contents are:\n");
    while (read(fd, &c, sizeof(c)) > 0)
        printf("%c", c);
    printf("\n");
    close(fd);
    return 0;
}
