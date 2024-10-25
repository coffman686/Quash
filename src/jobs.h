// jobs.h
#ifndef JOBS_H
#define JOBS_H

#include <sys/types.h>

#define MAX_INPUT 1024
#define MAX_JOBS 100

typedef struct {
    pid_t pid;
    int job_id;
    char command[MAX_INPUT];
} Job;

void list_jobs();
void handle_background_jobs();

extern Job jobs[MAX_JOBS]; // Declare the array of jobs
extern int job_count;      // Declare the job count

#endif
