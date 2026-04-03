#include <stdio.h>
#include <pthread.h>

void* func(void* arg) {
    printf("Thread Running\n");
    return NULL;
}

int main() {
    pthread_t t1;
    pthread_create(&t1, NULL, func, NULL);
    pthread_join(t1, NULL);
    return 0;
}

