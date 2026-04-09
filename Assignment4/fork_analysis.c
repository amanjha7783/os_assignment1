#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

int main() {
    pid_t pid = fork();

    if (pid == 0) {
        printf("[CHILD] PID=%d, Parent PID=%d\n", getpid(), getppid());
    } else {
        wait(NULL);
        printf("[PARENT] PID=%d, Child PID=%d\n", getpid(), pid);
    }

    printf("Both processes execute this line! PID=%d\n", getpid());

    return 0;
}
