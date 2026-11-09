#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>


#define NUM_PROCESSES 10
#define BLOCK_SIZE    5

int main(void)
{
    int fd, i;

    fd = open("output.txt", O_CREAT | O_TRUNC | O_RDWR, S_IRUSR | S_IWUSR);
    if (fd == -1) {
        perror("open");
        return 1;
    }
    
    /* Create child processes to write their blocks */
    for (i=1; i < NUM_PROCESSES; i++) {
        write(fd, "00000", BLOCK_SIZE); /* Write the block for the parent process */
        
        if (fork() == 0) {
            char text[BLOCK_SIZE + 1]; /* Buffer to hold the formatted string */
            snprintf(text, sizeof(text), "%05d", i*11111); /* Format the index as a 5-digit number */
            write(fd, text, BLOCK_SIZE); /* Write the block to the file */
            exit(0);        /* termina el proceso hijo */
        }
        wait(NULL); /* Wait for the child process to finish before continuing */
    }

    /* Read and print the contents of the file */
    char c;
    lseek(fd, 0, SEEK_SET);
    printf("File contents are:\n");
    while (read(fd, &c, sizeof(c)) > 0)
        printf("%c", c);
    printf("\n");
    close(fd);
    return(0);
}
