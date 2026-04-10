#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MAX 100

typedef struct {
    int pid, arrival, burst, remaining, priority;
    int start, completion, waiting, turnaround, response;
    int visited;
} Process;

Process original[MAX], p[MAX];
int n;

float cpuUtil[10];
char *names[] = {"FCFS","RR-2","RR-4","SJF","SRTN","PRIORITY"};

// -----------------------------
void copy_processes() {
    for(int i=0;i<n;i++)
        p[i] = original[i];
}

// -----------------------------
void generate_processes() {
    srand(time(0));
    for(int i=0;i<n;i++) {
        original[i].pid = i;
        original[i].arrival = rand() % 10;
        original[i].burst = (rand() % 9) + 1;
        original[i].remaining = original[i].burst;
        original[i].priority = rand() % 5 + 1;

        original[i].start = -1;
        original[i].visited = 0;
    }
}

// -----------------------------
void print_gantt_line(int time) {
    printf("\nTime: ");
    for(int i=0;i<=time;i++)
        printf("%-4d", i);
    printf("\n");
}

// -----------------------------
float calculate_metrics(Process proc[]) {
    float wt=0, tat=0, rt=0;

    printf("\n-------------------------------------------------------------\n");
    printf("%-5s %-5s %-5s %-5s %-5s %-5s %-5s\n",
           "PID","AT","BT","CT","WT","TAT","RT");
    printf("-------------------------------------------------------------\n");

    for(int i=0;i<n;i++) {
        proc[i].turnaround = proc[i].completion - proc[i].arrival;
        proc[i].waiting = proc[i].turnaround - proc[i].burst;

        wt += proc[i].waiting;
        tat += proc[i].turnaround;
        rt += proc[i].response;

        printf("%-5d %-5d %-5d %-5d %-5d %-5d %-5d\n",
            proc[i].pid, proc[i].arrival, proc[i].burst,
            proc[i].completion, proc[i].waiting,
            proc[i].turnaround, proc[i].response);
    }

    printf("-------------------------------------------------------------\n");
    printf("Avg WT: %-8.2f Avg TAT: %-8.2f Avg RT: %-8.2f\n",
           wt/n, tat/n, rt/n);
    printf("-------------------------------------------------------------\n");

    return wt/n;
}

// -----------------------------
float cpu_util(int idle, int total) {
    return ((float)(total - idle) / total) * 100;
}

// -----------------------------
// FCFS
// -----------------------------
void fcfs(int idx) {
    copy_processes();
    int time=0, idle=0;

    for(int i=0;i<n-1;i++)
        for(int j=i+1;j<n;j++)
            if(p[i].arrival > p[j].arrival) {
                Process t = p[i]; p[i]=p[j]; p[j]=t;
            }

    printf("\n\n==================== FCFS ====================\n");
    printf("Gantt Chart:\n");

    for(int i=0;i<n;i++) {
        if(time < p[i].arrival) {
            idle += (p[i].arrival - time);
            time = p[i].arrival;
        }

        p[i].start = time;
        p[i].response = time - p[i].arrival;

        for(int j=0;j<p[i].burst;j++)
            printf("| P%-2d ", p[i].pid);

        time += p[i].burst;
        p[i].completion = time;
    }

    printf("|\n");
    print_gantt_line(time);

    calculate_metrics(p);
    cpuUtil[idx] = cpu_util(idle, time);

    printf("\nCPU Utilization : %.2f%%\n", cpuUtil[idx]);
    printf("====================================================\n");
}

// -----------------------------
// SJF
// -----------------------------
void sjf(int idx) {
    copy_processes();
    int time=0, completed=0, idle=0;

    printf("\n\n==================== SJF ====================\n");
    printf("Gantt Chart:\n");

    while(completed < n) {
        int idxp=-1, min=1e9;

        for(int i=0;i<n;i++)
            if(p[i].arrival<=time && !p[i].visited && p[i].burst<min) {
                min=p[i].burst;
                idxp=i;
            }

        if(idxp!=-1) {
            p[idxp].start=time;
            p[idxp].response=time-p[idxp].arrival;

            for(int j=0;j<p[idxp].burst;j++)
                printf("| P%-2d ", p[idxp].pid);

            time+=p[idxp].burst;
            p[idxp].completion=time;
            p[idxp].visited=1;
            completed++;
        } else {
            idle++; time++;
        }
    }

    printf("|\n");
    print_gantt_line(time);

    calculate_metrics(p);
    cpuUtil[idx]=cpu_util(idle,time);

    printf("\nCPU Utilization : %.2f%%\n", cpuUtil[idx]);
    printf("====================================================\n");
}

// -----------------------------
// SRTN
// -----------------------------
void srtn(int idx) {
    copy_processes();
    int time=0, completed=0, idle=0;

    printf("\n\n==================== SRTN ====================\n");
    printf("Gantt Chart:\n");

    while(completed<n) {
        int idxp=-1, min=1e9;

        for(int i=0;i<n;i++)
            if(p[i].arrival<=time && p[i].remaining>0 && p[i].remaining<min) {
                min=p[i].remaining;
                idxp=i;
            }

        if(idxp!=-1) {
            if(p[idxp].start==-1) {
                p[idxp].start=time;
                p[idxp].response=time-p[idxp].arrival;
            }

            printf("| P%-2d ", p[idxp].pid);
            p[idxp].remaining--;
            time++;

            if(p[idxp].remaining==0) {
                p[idxp].completion=time;
                completed++;
            }
        } else {
            idle++; time++;
        }
    }

    printf("|\n");
    print_gantt_line(time);

    calculate_metrics(p);
    cpuUtil[idx]=cpu_util(idle,time);

    printf("\nCPU Utilization : %.2f%%\n", cpuUtil[idx]);
    printf("====================================================\n");
}

// -----------------------------
// Round Robin
// -----------------------------
void rr(int q, int idx) {
    copy_processes();
    int queue[MAX], front=0,rear=0;
    int time=0, completed=0, idle=0;
    int inQ[MAX]={0};

    printf("\n\n==================== RR (q=%d) ====================\n",q);
    printf("Gantt Chart:\n");

    while(completed<n) {
        for(int i=0;i<n;i++)
            if(p[i].arrival<=time && !inQ[i] && p[i].remaining>0) {
                queue[rear++]=i;
                inQ[i]=1;
            }

        if(front==rear) {
            idle++; time++; continue;
        }

        int i=queue[front++];

        if(p[i].start==-1) {
            p[i].start=time;
            p[i].response=time-p[i].arrival;
        }

        int run=(p[i].remaining<q)?p[i].remaining:q;

        for(int j=0;j<run;j++)
            printf("| P%-2d ", p[i].pid);

        p[i].remaining-=run;
        time+=run;

        for(int j=0;j<n;j++)
            if(p[j].arrival<=time && !inQ[j] && p[j].remaining>0) {
                queue[rear++]=j;
                inQ[j]=1;
            }

        if(p[i].remaining>0)
            queue[rear++]=i;
        else {
            p[i].completion=time;
            completed++;
        }
    }

    printf("|\n");
    print_gantt_line(time);

    calculate_metrics(p);
    cpuUtil[idx]=cpu_util(idle,time);

    printf("\nCPU Utilization : %.2f%%\n", cpuUtil[idx]);
    printf("====================================================\n");
}

// -----------------------------
// Priority
// -----------------------------
void priority_sched(int idx) {
    copy_processes();
    int time=0, completed=0, idle=0;

    printf("\n\n==================== PRIORITY ====================\n");
    printf("Gantt Chart:\n");

    while(completed<n) {
        int idxp=-1, min=1e9;

        for(int i=0;i<n;i++)
            if(p[i].arrival<=time && !p[i].visited && p[i].priority<min) {
                min=p[i].priority;
                idxp=i;
            }

        if(idxp!=-1) {
            p[idxp].start=time;
            p[idxp].response=time-p[idxp].arrival;

            for(int j=0;j<p[idxp].burst;j++)
                printf("| P%-2d ", p[idxp].pid);

            time+=p[idxp].burst;
            p[idxp].completion=time;
            p[idxp].visited=1;
            completed++;
        } else {
            idle++; time++;
        }
    }

    printf("|\n");
    print_gantt_line(time);

    calculate_metrics(p);
    cpuUtil[idx]=cpu_util(idle,time);

    printf("\nCPU Utilization : %.2f%%\n", cpuUtil[idx]);
    printf("====================================================\n");
}

// -----------------------------
// Comparison
// -----------------------------
void comparison() {
    printf("\n\n================ FINAL COMPARISON ================\n");
    printf("%-12s %-10s\n", "Algorithm", "CPU(%)");
    printf("---------------------------------------------\n");

    for(int i=0;i<6;i++)
        printf("%-12s %-10.2f\n", names[i], cpuUtil[i]);

    printf("===============================================\n");
}

// -----------------------------
// MAIN MENU
// -----------------------------
int main() {
    int choice;

    printf("Enter number of processes: ");
    scanf("%d", &n);

    generate_processes();

    while(1) {
        printf("\n\n=========== CPU SCHEDULING MENU ===========\n");
        printf("1. FCFS\n");
        printf("2. Round Robin (q=2)\n");
        printf("3. Round Robin (q=4)\n");
        printf("4. SJF\n");
        printf("5. SRTN\n");
        printf("6. Priority\n");
        printf("7. Run ALL + Compare\n");
        printf("0. Exit\n");
        printf("Enter choice: ");
        scanf("%d", &choice);

        switch(choice) {
            case 1: fcfs(0); break;
            case 2: rr(2,1); break;
            case 3: rr(4,2); break;
            case 4: sjf(3); break;
            case 5: srtn(4); break;
            case 6: priority_sched(5); break;
            case 7:
                fcfs(0);
                rr(2,1);
                rr(4,2);
                sjf(3);
                srtn(4);
                priority_sched(5);
                comparison();
                break;
            case 0:
                printf("Exiting...\n");
                return 0;
            default:
                printf("Invalid choice!\n");
        }
    }
}
