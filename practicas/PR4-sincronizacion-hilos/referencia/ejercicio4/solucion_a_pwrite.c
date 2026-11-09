/* Cuestión A, variante con pwrite(): escribe en una posición indicada SIN tocar el
 * desplazamiento del descriptor, así que se puede compartir el mismo fd sin problemas. */
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
        if (fork() == 0) {
            char text[BLOCK_SIZE + 1];
            snprintf(text, sizeof(text), "%05d", i * 11111);
            pwrite(fd, text, BLOCK_SIZE, i * BLOCK_SIZE);
            exit(0);
        }
    }
    pwrite(fd, "00000", BLOCK_SIZE, 0);

    while (wait(NULL) != -1)
        ;

    char c;
    lseek(fd, 0, SEEK_SET);
    printf("File contents are:\n");
    while (read(fd, &c, sizeof(c)) > 0)
        printf("%c", c);
    printf("\n");
    close(fd);
    return 0;
}
