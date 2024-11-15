#include "userapp.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

void register_process(unsigned int pid) {
    FILE *proc_file = fopen("/proc/kmlab/status", "w");
    if (proc_file) {
        fprintf(proc_file, "%u", pid);
        fclose(proc_file);
    } else {
        perror("Error opening /proc/kmlab/status");
    }
}

int main(int argc, char *argv[]) {
    int expire = 10;
    time_t start_time = time(NULL);

    if (argc == 2) expire = atoi(argv[1]);

    register_process(getpid());

    while (1) {
        if ((int)(time(NULL) - start_time) > expire) break;
    }

    return 0;
}
