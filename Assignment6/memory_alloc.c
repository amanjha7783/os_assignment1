/* memory_alloc.c — First Fit, Best Fit, Worst Fit */
#include <stdio.h>

#define MAX_BLOCKS 10
#define MAX_PROC   10

// -------- FIRST FIT --------
void first_fit(int blocks[], int nb, int procs[], int np) {
    int alloc[MAX_PROC], avail[MAX_BLOCKS];

    // Initialize
    for(int i = 0; i < nb; i++) avail[i] = blocks[i];
    for(int i = 0; i < np; i++) alloc[i] = -1;

    printf("\n--- First Fit ---\n");

    for(int i = 0; i < np; i++) {
        for(int j = 0; j < nb; j++) {
            if(avail[j] >= procs[i]) {
                alloc[i] = j;
                avail[j] -= procs[i];
                break;
            }
        }
    }

    for(int i = 0; i < np; i++) {
        printf("Process %d (%dKB) -> Block %d\n",
               i+1, procs[i], alloc[i] != -1 ? alloc[i]+1 : 0);
    }
}

// -------- BEST FIT --------
void best_fit(int blocks[], int nb, int procs[], int np) {
    int alloc[MAX_PROC], avail[MAX_BLOCKS];

    for(int i = 0; i < nb; i++) avail[i] = blocks[i];
    for(int i = 0; i < np; i++) alloc[i] = -1;

    printf("\n--- Best Fit ---\n");

    for(int i = 0; i < np; i++) {
        int best = -1;
        for(int j = 0; j < nb; j++) {
            if(avail[j] >= procs[i] &&
               (best == -1 || avail[j] < avail[best])) {
                best = j;
            }
        }
        if(best != -1) {
            alloc[i] = best;
            avail[best] -= procs[i];
        }
    }

    for(int i = 0; i < np; i++) {
        printf("Process %d (%dKB) -> Block %d\n",
               i+1, procs[i], alloc[i] != -1 ? alloc[i]+1 : 0);
    }
}

// -------- WORST FIT --------
void worst_fit(int blocks[], int nb, int procs[], int np) {
    int alloc[MAX_PROC], avail[MAX_BLOCKS];

    for(int i = 0; i < nb; i++) avail[i] = blocks[i];
    for(int i = 0; i < np; i++) alloc[i] = -1;

    printf("\n--- Worst Fit ---\n");

    for(int i = 0; i < np; i++) {
        int worst = -1;
        for(int j = 0; j < nb; j++) {
            if(avail[j] >= procs[i] &&
               (worst == -1 || avail[j] > avail[worst])) {
                worst = j;
            }
        }
        if(worst != -1) {
            alloc[i] = worst;
            avail[worst] -= procs[i];
        }
    }

    for(int i = 0; i < np; i++) {
        printf("Process %d (%dKB) -> Block %d\n",
               i+1, procs[i], alloc[i] != -1 ? alloc[i]+1 : 0);
    }
}

// -------- MAIN --------
int main() {
    int blocks[] = {100, 500, 200, 300, 600};
    int procs[]  = {212, 417, 112, 426};

    int nb = 5, np = 4;

    printf("Memory Blocks (KB): 100, 500, 200, 300, 600\n");
    printf("Process Sizes (KB): 212, 417, 112, 426\n");

    first_fit(blocks, nb, procs, np);
    best_fit(blocks, nb, procs, np);
    worst_fit(blocks, nb, procs, np);

    return 0;
}