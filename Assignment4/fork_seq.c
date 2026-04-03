#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char *argv[]) {

    // Check input
    if (argc != 2) {
        printf("Usage: %s <positive number>\n", argv[0]);
        return 1;
    }

    int num = atoi(argv[1]);

    // Validate positive integer
    if (num <= 0) {
        printf("Please enter a positive number\n");
        return 1;
    }

    int pid = fork();

    if (pid == 0) {
        // CHILD PROCESS
        printf("Child Process (PID: %d): ", getpid());

        while (num > 0) {
            printf("%d ", num);
            num = num / 2;
        }

        printf("\n");
        exit(0);
    }
    else {
        // PARENT PROCESS
        wait(NULL);
        printf("Parent Process (PID: %d) finished\n", getpid());
    }

    return 0;
}

