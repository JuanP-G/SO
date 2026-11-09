/* Cuestión A: escritura concurrente, sin sincronizar.
 * Cada proceso abre SU PROPIO descriptor (su propio desplazamiento) y escribe en la
 * posición que le corresponde (i * 5), sin leer nunca la posición de otro. */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>

#define NUM_PROCESSES 10
#define BLOCK_SIZE    5

static void write_block(int index)
{
    char text[BLOCK_SIZE + 1];
    snprintf(text, sizeof(text), "%05d", index * 11111);       /* 0 -> "00000", 1 -> "11111" */

    int fd = open("output.txt", O_WRONLY);                     /* descriptor propio */
    if (fd == -1) {
        perror("open");
        exit(1);
    }
    lseek(fd, index * BLOCK_SIZE, SEEK_SET);                   /* posición absoluta */
    write(fd, text, BLOCK_SIZE);
    close(fd);
}

int main(void)
{
    int fd = open("output.txt", O_CREAT | O_TRUNC | O_RDWR, S_IRUSR | S_IWUSR);
    if (fd == -1) {
        perror("open");
        return 1;
    }
    close(fd);                         /* el fichero ya existe y está vacío */

    for (int i = 1; i < NUM_PROCESSES; i++) {
        if (fork() == 0) {             /* hijo i */
            write_block(i);
            exit(0);
        }
    }
    write_block(0);                    /* el padre escribe el bloque 0 */

    while (wait(NULL) != -1)
        ;

    fd = open("output.txt", O_RDONLY);
    char c;
    printf("File contents are:\n");
    while (read(fd, &c, sizeof(c)) > 0)
        printf("%c", c);
    printf("\n");
    close(fd);
    return 0;
}
