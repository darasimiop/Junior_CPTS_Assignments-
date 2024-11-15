#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

typedef struct Process {
    uint32_t A;         // Arrival time
    uint32_t B;         // Upper bound of CPU burst
    uint32_t C;         // Total CPU time required
    uint32_t M;         // Multiplier for I/O burst
    uint32_t processID; // Process ID

    uint32_t remainingCPU;      // Remaining CPU time
    uint32_t finishingTime;     // Finishing time
    uint32_t turnaroundTime;    // Turnaround time
    uint32_t waitingTime;       // Waiting time
    uint32_t ioTime;            // Total time spent in I/O
} Process;

// Function prototypes
void FCFS(Process processes[], int count);
void RR(Process processes[], int count, uint32_t quantum);
void SJF(Process processes[], int count);
void printSummary(Process processes[], int count, const char *algorithmName);
int findNextProcess_SJF(Process processes[], int count, int currentTime);

void FCFS(Process processes[], int count) {
    printf("\nFirst-Come First-Serve Scheduling\n");
    int currentTime = 0;
    
    for (int i = 0; i < count; i++) {
        if (processes[i].A > currentTime) {
            currentTime = processes[i].A;
        }
        processes[i].waitingTime = currentTime - processes[i].A;
        currentTime += processes[i].remainingCPU;
        processes[i].finishingTime = currentTime;
        processes[i].turnaroundTime = processes[i].finishingTime - processes[i].A;
    }
    printSummary(processes, count, "FCFS");
}

void RR(Process processes[], int count, uint32_t quantum) {
    printf("\nRound Robin Scheduling\n");
    int currentTime = 0;
    bool done;
    int *remaining = (int *)malloc(count * sizeof(int));
    if (remaining == NULL) {
        fprintf(stderr, "Memory allocation failed.\n");
        return;
    }

    for (int i = 0; i < count; i++) {
        remaining[i] = processes[i].remainingCPU;
    }

    do {
        done = true;
        for (int i = 0; i < count; i++) {
            if (processes[i].A <= currentTime && remaining[i] > 0) {
                done = false;
                if (remaining[i] > quantum) {
                    remaining[i] -= quantum;
                    currentTime += quantum;
                } else {
                    currentTime += remaining[i];
                    processes[i].finishingTime = currentTime;
                    processes[i].turnaroundTime = currentTime - processes[i].A;
                    processes[i].waitingTime = processes[i].turnaroundTime - processes[i].C;
                    remaining[i] = 0;
                }
            }
        }
    } while (!done);

    free(remaining);
    printSummary(processes, count, "RR");
}

void SJF(Process processes[], int count) {
    printf("\nShortest Job First Scheduling\n");
    int currentTime = 0;
    bool done;

    for (int i = 0; i < count; i++) {
        int nextProcess = findNextProcess_SJF(processes, count, currentTime);
        if (nextProcess == -1) {
            currentTime++;
            i--; // Recheck the condition in the next cycle
            continue;
        }

        if (processes[nextProcess].A > currentTime) {
            currentTime = processes[nextProcess].A;
        }
        
        processes[nextProcess].waitingTime = currentTime - processes[nextProcess].A;
        currentTime += processes[nextProcess].remainingCPU;
        processes[nextProcess].finishingTime = currentTime;
        processes[nextProcess].turnaroundTime = processes[nextProcess].finishingTime - processes[nextProcess].A;
        processes[nextProcess].remainingCPU = 0;
    }

    printSummary(processes, count, "SJF");
}

int findNextProcess_SJF(Process processes[], int count, int currentTime) {
    int shortestJobIndex = -1;
    for (int i = 0; i < count; i++) {
        if (processes[i].remainingCPU > 0 && processes[i].A <= currentTime) {
            if (shortestJobIndex == -1 || processes[i].remainingCPU < processes[shortestJobIndex].remainingCPU) {
                shortestJobIndex = i;
            }
        }
    }
    return shortestJobIndex;
}

void printSummary(Process processes[], int count, const char *algorithmName) {
    int totalTurnaroundTime = 0;
    int totalWaitingTime = 0;
    int totalTime = 0;
    printf("\nSummary for %s:\n", algorithmName);

    for (int i = 0; i < count; i++) {
        printf("Process %d: Arrival=%d, CPU=%d, Finishing=%d, Turnaround=%d, Waiting=%d\n",
               processes[i].processID, processes[i].A, processes[i].C,
               processes[i].finishingTime, processes[i].turnaroundTime, processes[i].waitingTime);

        totalTurnaroundTime += processes[i].turnaroundTime;
        totalWaitingTime += processes[i].waitingTime;
        if (processes[i].finishingTime > totalTime) {
            totalTime = processes[i].finishingTime;
        }
    }

    double avgTurnaroundTime = (double)totalTurnaroundTime / count;
    double avgWaitingTime = (double)totalWaitingTime / count;
    printf("Total Time: %d\n", totalTime);
    printf("Average Turnaround Time: %.2f\n", avgTurnaroundTime);
    printf("Average Waiting Time: %.2f\n\n", avgWaitingTime);
}

int main() {
    Process processes[3] = {
        {0, 3, 5, 1, 1, 5, 0, 0, 0, 0},
        {1, 2, 4, 1, 2, 4, 0, 0, 0, 0},
        {2, 4, 3, 1, 3, 3, 0, 0, 0, 0}
    };

    // Run each algorithm on a fresh copy of the process list
    FCFS(processes, 3);

    // Resetting the processes for the next algorithm
    for (int i = 0; i < 3; i++) {
        processes[i].remainingCPU = processes[i].C;
    }
    RR(processes, 3, 2);

    // Resetting the processes for the next algorithm
    for (int i = 0; i < 3; i++) {
        processes[i].remainingCPU = processes[i].C;
    }
    SJF(processes, 3);

    return 0;
}
