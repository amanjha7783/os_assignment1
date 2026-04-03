#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

int main() {
    int pid = fork();

    if (pid == 0) {
        // child process
        printf("Child Process\n");
        execl("/bin/ls", "ls", NULL);
    } else {
        // parent process
        wait(NULL);
        printf("Parent Process\n");
    }

    return 0;
}

