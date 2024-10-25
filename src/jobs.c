#include "quash.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "jobs.h"


void handle_background_jobs() {
    int status;
    pid_t pid;
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        for (int i = 0; i < job_count; i++) {
            if (jobs[i].pid == pid) {
                printf("Completed: [%d] %d %s \n", jobs[i].job_id, pid, jobs[i].command);
                jobs[i].pid = -1;  // Mark as completed
                break;
            }
        }
    }
}

void list_jobs() {
    printf("Current Background Jobs:\n");
    for (int i = 0; i < job_count; i++) {
        if (jobs[i].pid > 0) { // Only print active jobs
            printf("[%d] %d %s \n", jobs[i].job_id, jobs[i].pid, jobs[i].command);
        }
    }
}