//header files
#include "quash.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "jobs.h"

//Function to handle background jobs that have completed
void handle_background_jobs() { 
    int status; //store status information of a child process
    pid_t pid; //store process IDs of completed child processes
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {//check for completed background jobs w/o blocking
        for (int i = 0; i < job_count; i++) { //loop through all jobs to find matching PID
            if (jobs[i].pid == pid) { //check if PID matches a background job
                printf("Completed: [%d] %d %s \n", jobs[i].job_id, pid, jobs[i].command); // output job completion info
                jobs[i].pid = -1;  // Mark as completed through -1
                break; // exit loop when job is found & marked
            }
        }
    }
}
//Function to list all active background jobs
void list_jobs() {
    printf("Current Background Jobs:\n"); //print header for job listing
    for (int i = 0; i < job_count; i++) { //loop thru all jobs
        if (jobs[i].pid > 0) { // Only print active jobs
            printf("[%d] %d %s \n", jobs[i].job_id, jobs[i].pid, jobs[i].command); //Print job ID PID and command
        }
    }
}
