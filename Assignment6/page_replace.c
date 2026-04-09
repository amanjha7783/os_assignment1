/* page_replace.c — FIFO and LRU page replacement */
#include <stdio.h>

#define FRAMES 3
#define PAGES  12

// -------- FIFO --------
void fifo(int ref[], int n) {
    int frames[FRAMES], front = 0, faults = 0;

    // Initialize frames
    for(int i = 0; i < FRAMES; i++)
        frames[i] = -1;

    printf("\n=== FIFO Page Replacement ===\n");

    for(int i = 0; i < n; i++) {
        int hit = 0;

        // Check if page already in frames
        for(int j = 0; j < FRAMES; j++) {
            if(frames[j] == ref[i]) {
                hit = 1;
                break;
            }
        }

        // If not found → replace
        if(!hit) {
            frames[front] = ref[i];
            front = (front + 1) % FRAMES;
            faults++;
        }

        printf("Page %2d: [%2d %2d %2d] %s\n",
               ref[i], frames[0], frames[1], frames[2],
               hit ? "HIT" : "FAULT");
    }

    printf("Total Page Faults: %d\n", faults);
}

// -------- LRU --------
void lru(int ref[], int n) {
    int frames[FRAMES], time[FRAMES], faults = 0;

    // Initialize
    for(int i = 0; i < FRAMES; i++) {
        frames[i] = -1;
        time[i] = 0;
    }

    printf("\n=== LRU Page Replacement ===\n");

    for(int i = 0; i < n; i++) {
        int hit = -1;

        // Check hit
        for(int j = 0; j < FRAMES; j++) {
            if(frames[j] == ref[i]) {
                hit = j;
                break;
            }
        }

        if(hit == -1) {
            // Find least recently used
            int lru_idx = 0;
            for(int j = 1; j < FRAMES; j++) {
                if(time[j] < time[lru_idx])
                    lru_idx = j;
            }

            frames[lru_idx] = ref[i];
            time[lru_idx] = i + 1;
            faults++;
        } else {
            // Update time if hit
            time[hit] = i + 1;
        }

        printf("Page %2d: [%2d %2d %2d] %s\n",
               ref[i], frames[0], frames[1], frames[2],
               hit == -1 ? "FAULT" : "HIT");
    }

    printf("Total Page Faults: %d\n", faults);
}

// -------- MAIN --------
int main() {
    int ref[] = {7,0,1,2,0,3,0,4,2,3,0,3};

    printf("Frames = %d | Reference string: 7,0,1,2,0,3,0,4,2,3,0,3\n", FRAMES);

    fifo(ref, PAGES);
    lru(ref, PAGES);

    return 0;
}
