#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#include "jobs.h"

/* variables */
int mode;
pid_t groupID;

int activeJobs;
JobsList *jobsList;

/* function implementations */
JobsList *
add_job(pid_t pgid, char *name, int status) 
{
    JobsList *newJob = malloc(sizeof(JobsList));
    newJob->name = (char *) malloc(sizeof(name));
    newJob->name = strcpy(newJob->name, name);
    newJob->pgid = pgid;
    newJob->status = status;
    newJob->next = NULL;

    if(jobsList == NULL) {
        activeJobs++;
        newJob->id = activeJobs;
        return newJob;
    } else {
        JobsList *tmpList = jobsList;
        while (tmpList->next != NULL) {
            tmpList = tmpList->next;
        }
        newJob->id = tmpList->id + 1;
        tmpList->next = newJob;
        activeJobs++;
        return jobsList;
    }
}

JobsList *
del_job(JobsList *job) 
{
    if(jobsList == NULL) {
        return NULL;
    }
    JobsList *currentJob;
    JobsList *prevJob;

    currentJob = jobsList->next;
    prevJob = jobsList;

    if(prevJob->pgid == job->pgid) {
        prevJob = prevJob->next;
        activeJobs--;
        return currentJob;
    }

    while (currentJob != NULL) {
         if(currentJob->pgid == job->pgid) {
              activeJobs--;
              prevJob->next = currentJob->next;
         }
         prevJob = currentJob;
         currentJob = currentJob->next;
    }
    return jobsList;
}

JobsList *
get_job(int key, int searchParameter) 
{
    JobsList *job = jobsList;
    if(searchParameter == 1) {
       while (job != NULL) {
            if(job->pgid == key) {
                return job;
            } else {
                job = job->next;
            }
        }
    }
    else if(searchParameter == 0) {
        while (job != NULL) {
            if(job->id == key) {
                return job;
            } else {
                job = job->next;
            }
        }
    }
    
    return NULL;
}

int 
change_job_status(int pid, int status) 
{
    if(jobsList == NULL) {
        return 0;
    } else {
        JobsList *job = jobsList;
        while (job != NULL) {
            if(job->pgid == pid) {
                job->status = status;
                return 1;
            }
            job = job->next;
        }
        return 0;
    }
}

void 
wait_job(JobsList *job) 
{
    int terminationStatus;
    while (waitpid(job->pgid, &terminationStatus, WNOHANG) == 0) {
        if(job->status == SUSPENDED) {
            return;
        }
    }
    jobsList = del_job(job);
}


void 
kill_job(int jobID) 
{
    if(jobsList != NULL) {
        JobsList *job = get_job(jobID, 0);
        kill(job->pgid, SIGKILL);
    }   
}

void 
put_job_foreground(JobsList *job, int continueJob) 
{
    if(job == NULL) {
        return;
    }
    job->status = FOREGROUND;
    tcsetpgrp(STDIN_FILENO, job->pgid);
    if(continueJob) {
        if(kill(job->pgid, SIGCONT) < 0) {
            perror("kill (SIGCONT)");
        }
    }

    wait_job(job);
    tcsetpgrp(STDIN_FILENO, groupID);
}

void 
put_job_background(JobsList *job, int continueJob) 
{
    if(job == NULL) {
        return;
    }
    job->status = BACKGROUND;
    if(continueJob) {
        if(kill(job->pgid, SIGCONT) < 0) {
            perror("kill (SIGCONT)");
        }
    }
    tcsetpgrp(STDIN_FILENO, groupID);
}

void 
signal_handler_child() 
{
    pid_t pid;
    int terminationStatus;
    pid = waitpid(WAIT_ANY, &terminationStatus, WUNTRACED | WNOHANG);
    if (pid > 0) {
        JobsList *job = get_job(pid, 1);
        if (job == NULL) {
            return;
        }

        if (WIFEXITED(terminationStatus)) {
            if (job->status == BACKGROUND) {
                printf("\n[%d]+  Done\t   %s\n", job->id, job->name);
                jobsList = del_job(job);
            }
        }
        else if (WIFSIGNALED(terminationStatus)) {
            printf("\n[%d]+  KILLED\t   %s\n", job->id, job->name);
            jobsList = del_job(job);
        }
        else if (WIFSTOPPED(terminationStatus)) {
            tcsetpgrp(STDIN_FILENO, job->pgid);
            change_job_status(pid, SUSPENDED);
            printf("\n[%d]+   stopped\t   %s\n", activeJobs, job->name);
            return;
        } else {
            if (job->status == BACKGROUND) {
                jobsList = del_job(job);
            }
        }
    }
}

void 
start_job(char *command[], int executionMode) 
{
    pid_t pid;
    if((pid = fork()) == -1) {
        perror("fork error");
        exit(EXIT_FAILURE);
    }
    else if(pid == 0) {
        signal(SIGINT, SIG_DFL);
        signal(SIGQUIT, SIG_DFL);
        signal(SIGTSTP, SIG_DFL);
        signal(SIGCHLD, &signal_handler_child);
        signal(SIGTTIN, SIG_DFL);
        setpgrp();
        if(executionMode == FOREGROUND) {
            tcsetpgrp(STDIN_FILENO, getpid());
        }

        if(executionMode == BACKGROUND) {
            printf("[%d] %d\n", ++activeJobs, (int) getpid());
        }

        if(execvp(*command, command) == -1) {
            printf("Error. Command not found: %s\n", command[0]);
        }
            
        exit(EXIT_SUCCESS);
    } else {
        setpgid(pid, pid);
        jobsList = add_job(pid, *(command), (int) executionMode);
        JobsList *job = get_job(pid, 1);
        if(executionMode == FOREGROUND) {
             put_job_foreground(job, 0);
        }

        if(executionMode == BACKGROUND) {
            put_job_background(job, 0);
        }
    }

}

void 
print_jobs() 
{
    JobsList *job = jobsList;
    while (job != NULL) {
        printf("%d\t%c\t%s\t%d\n", job->id, job->status, job->name, job->pgid);
        job = job->next;
     }
}
