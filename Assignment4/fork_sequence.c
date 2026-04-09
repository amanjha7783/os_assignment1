/* fork_sequence.c — child generates halving sequence */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>    /* fork(), getpid() */
#include <sys/wait.h>   /* wait() */

int main(int argc, char *argv[]) {
    /* Validate command-line argument */
    if (argc != 2) {
        printf("Usage: %s <positive_integer>\n", argv[0]);
        exit(1);
    }

    int n = atoi(argv[1]);
    if (n <= 0) {
        printf("Error: Enter a positive integer.\n");
        exit(1);
    }

    printf("Parent PID: %d — creating child...\n", getpid());

    pid_t pid = fork();   /* create child process */

    if (pid < 0) {
        perror("fork failed"); exit(1);

    } else if (pid == 0) {
        /* ── CHILD PROCESS ── */
        printf("Child PID: %d — Sequence: ", getpid());
        while (n >= 1) {
            printf("%d", n);
            n /= 2;
            if (n >= 1) printf(", ");
        }
        printf("\n");
        exit(0);   /* child exits */

    } else {
        /* ── PARENT PROCESS ── */
        wait(NULL);   /* wait for child to finish */
        printf("Parent: child done. Exiting.\n");
    }

    return 0;
}
