/* cpu_scheduler.c — FCFS, SJF, RR, Priority scheduling simulator */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX 50

typedef struct {
    int pid, arrival, burst, priority;
    int waiting, turnaround, response, remaining, start;
    int started;
} Process;

Process procs[MAX];
int n;

/* ── Random data generator ── */
void generate_processes(int count) {
    n = count;
    srand(time(NULL));
    printf("\n%-5s %-10s %-12s %-10s\n",
           "PID", "Arrival", "Burst", "Priority");
    printf("%-5s %-10s %-12s %-10s\n",
           "---", "-------", "-----", "--------");
    for (int i = 0; i < n; i++) {
        procs[i].pid      = i + 1;
        procs[i].arrival  = rand() % 10;
        procs[i].burst    = rand() % 10 + 1;
        procs[i].priority = rand() % 5 + 1;
        procs[i].remaining = procs[i].burst;
        procs[i].started  = 0;
        printf("P%-4d %-10d %-12d %-10d\n",
               procs[i].pid, procs[i].arrival,
               procs[i].burst, procs[i].priority);
    }
}

void reset() {
    for (int i = 0; i < n; i++) {
        procs[i].waiting = procs[i].turnaround = procs[i].response = 0;
        procs[i].remaining = procs[i].burst;
        procs[i].started = 0;
    }
}

void print_results(char *algo) {
    float avg_wt = 0, avg_rt = 0, avg_tat = 0;
    printf("\n=== %s ===\n", algo);
    printf("%-5s %-10s %-12s %-12s\n",
           "PID", "Wait", "Response", "Turnaround");
    for (int i = 0; i < n; i++) {
        printf("P%-4d %-10d %-12d %-12d\n",
               procs[i].pid, procs[i].waiting,
               procs[i].response, procs[i].turnaround);
        avg_wt  += procs[i].waiting;
        avg_rt  += procs[i].response;
        avg_tat += procs[i].turnaround;
    }
    printf("Avg Wait=%.2f  Avg Response=%.2f  Avg Turnaround=%.2f\n",
           avg_wt/n, avg_rt/n, avg_tat/n);
}

/* ── FCFS (sort by arrival, run in order) ── */
void fcfs() {
    reset();
    int time = 0, done[MAX] = {0};
    for (int c = 0; c < n; c++) {
        int sel = -1, minArr = 99999;
        for (int i = 0; i < n; i++)
            if (!done[i] && procs[i].arrival < minArr) {
                minArr = procs[i].arrival; sel = i;
            }
        if (time < procs[sel].arrival) time = procs[sel].arrival;
        procs[sel].response   = time - procs[sel].arrival;
        procs[sel].waiting    = time - procs[sel].arrival;
        time                 += procs[sel].burst;
        procs[sel].turnaround = time - procs[sel].arrival;
        done[sel] = 1;
    }
    print_results("FCFS — First Come First Served");
}

/* ── SJF Non-preemptive ── */
void sjf() {
    reset();
    int time = 0, done[MAX] = {0}, completed = 0;
    while (completed < n) {
        int sel = -1, minBurst = 99999;
        for (int i = 0; i < n; i++)
            if (!done[i] && procs[i].arrival <= time && procs[i].burst < minBurst) {
                minBurst = procs[i].burst; sel = i;
            }
        if (sel == -1) { time++; continue; }
        procs[sel].waiting    = time - procs[sel].arrival;
        procs[sel].response   = procs[sel].waiting;
        time                 += procs[sel].burst;
        procs[sel].turnaround = time - procs[sel].arrival;
        done[sel] = 1; completed++;
    }
    print_results("SJF — Shortest Job First (Non-Preemptive)");
}

/* ── Round Robin ── */
void round_robin(int quantum) {
    reset();
    int time = 0, done[MAX] = {0}, completed = 0;
    int queue[MAX*100], front = 0, rear = 0;
    int inQueue[MAX] = {0};

    for (int i = 0; i < n; i++)
        if (procs[i].arrival == 0) { queue[rear++] = i; inQueue[i] = 1; }

    while (completed < n) {
        if (front == rear) { time++; continue; }
        int i = queue[front++];
        if (!procs[i].started) { procs[i].response = time - procs[i].arrival; procs[i].started = 1; }
        int run = (procs[i].remaining < quantum) ? procs[i].remaining : quantum;
        procs[i].remaining -= run; time += run;
        for (int j = 0; j < n; j++)
            if (!done[j] && !inQueue[j] && procs[j].arrival <= time)
                { queue[rear++] = j; inQueue[j] = 1; }
        if (procs[i].remaining > 0) queue[rear++] = i;
        else {
            procs[i].turnaround = time - procs[i].arrival;
            procs[i].waiting    = procs[i].turnaround - procs[i].burst;
            done[i] = 1; completed++;
        }
    }
    char title[50];
    sprintf(title, "Round Robin (Quantum=%d)", quantum);
    print_results(title);
}

/* ── Priority (Non-preemptive) ── */
void priority_np() {
    reset();
    int time = 0, done[MAX] = {0}, completed = 0;
    while (completed < n) {
        int sel = -1, minPri = 99999;
        for (int i = 0; i < n; i++)
            if (!done[i] && procs[i].arrival <= time && procs[i].priority < minPri) {
                minPri = procs[i].priority; sel = i;
            }
        if (sel == -1) { time++; continue; }
        procs[sel].response   = time - procs[sel].arrival;
        procs[sel].waiting    = procs[sel].response;
        time                 += procs[sel].burst;
        procs[sel].turnaround = time - procs[sel].arrival;
        done[sel] = 1; completed++;
    }
    print_results("Priority Scheduling (Non-Preemptive, lower=higher priority)");
}

int main() {
    int algo, nproc, quantum;
    printf("\nWelcome to CPU Scheduling Simulator\n");
    printf("Scheduling Algorithm: (1)FCFS (2)RR (3)SJF (4)Priority (5)All\n");
    scanf("%d", &algo);
    printf("Number of processes: ");
    scanf("%d", &nproc);
    if (nproc > MAX) nproc = MAX;

    printf("\nGenerating %d random processes...\n", nproc);
    generate_processes(nproc);

    switch(algo) {
        case 1: fcfs(); break;
        case 2:
            printf("Time quantum: "); scanf("%d", &quantum);
            round_robin(quantum); break;
        case 3: sjf(); break;
        case 4: priority_np(); break;
        case 5:
            fcfs(); sjf(); priority_np();
            round_robin(2); round_robin(4); break;
        default: printf("Invalid option\n");
    }
    return 0;
}
