#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>


#define NUM_PROCESSES 10
#define BLOCK_SIZE    5

static void write_block(int index)
{
    char text[BLOCK_SIZE + 1]; /* Buffer to hold the formatted string */
    snprintf(text, sizeof(text), "%05d", index*11111); /* Format the index as a 5-digit number */

    /* Open the file in write-only mode */
    int fd = open("output.txt", O_WRONLY);
    if (fd == -1) {
        perror("open");
        exit(1);
    }
    /* Move the file pointer to the correct position and write the block */
    lseek(fd, index * BLOCK_SIZE, SEEK_SET);
    write(fd, text, BLOCK_SIZE);
    close(fd);
}

int main(void)
{
    int fd, i;

    fd = open("output.txt", O_CREAT | O_TRUNC | O_RDWR, S_IRUSR | S_IWUSR);
    if (fd == -1) {
        perror("open");
        return 1;
    }
    close(fd);                         /* el fichero ya existe y está vacío */

    /* Create child processes to write their blocks */
    for (i=1; i < NUM_PROCESSES; i++) {
        if (fork() == 0) {
            write_block(i); /* escribe su bloque en el fichero */
            exit(0);        /* termina el proceso hijo */
        }
    }
    /* Write the block for the parent process */
    write_block(0);

	/* Wait for all child processes to finish */
    while (wait(NULL) != -1);

    /* Read and print the contents of the file */
    fd = open("output.txt", O_RDONLY);
    printf("File contents are:\n");
    char c;
    while (read(fd, &c, sizeof(c)) > 0)
        printf("%c", c);
    printf("\n");
    close(fd);
    return(0);
}
