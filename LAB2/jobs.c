#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#include "jobs.h"

/* variables */
int active_jobs;
JobsList *jobs_list;

/* function implementations */

JobsList *
add_job(pid_t pgid, char *name, int status) 
{
    JobsList *new_job = malloc(sizeof(JobsList));
    new_job->name = (char *) malloc(1024);
    new_job->name = strcpy(new_job->name, name);
    new_job->pgid = pgid;
    new_job->status = status;
    new_job->next = NULL;

    if(jobs_list == NULL) {
        active_jobs++;
        new_job->id = active_jobs;
        return new_job;
    } else {
        JobsList *tmp_list = jobs_list;
        while (tmp_list->next != NULL) {
            tmp_list = tmp_list->next;
        }
        new_job->id = tmp_list->id + 1;
        tmp_list->next = new_job;
        active_jobs++;
        return jobs_list;
    }
}

JobsList *
del_job(JobsList *job) 
{
    if(jobs_list == NULL) {
        return NULL;
    }
    JobsList *current_job;
    JobsList *prev_job;

    current_job = jobs_list->next;
    prev_job = jobs_list;

    if(prev_job->pgid == job->pgid) {
        prev_job = prev_job->next;
        active_jobs--;
        return current_job;
    }

    while (current_job != NULL) {
         if(current_job->pgid == job->pgid) {
              active_jobs--;
              prev_job->next = current_job->next;
         }
         prev_job = current_job;
         current_job = current_job->next;
    }
    return jobs_list;
}

JobsList *
get_job(int key, int search_parameter) 
{
    JobsList *job = jobs_list;
    if(search_parameter == 1) {
       while (job != NULL) {
            if(job->pgid == key) {
                return job;
            } else {
                job = job->next;
            }
        }
    }
    else if(search_parameter == 0) {
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
    if(jobs_list == NULL) {
        return 0;
    } else {
        JobsList *job = jobs_list;
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
    int termination_status;

    /* "pid_t waitpid (pid_t pid, int *status-ptr, int options)"
     *
     * this function is used to request status information from a child 
     * process. Normally, the calling process is suspended until the 
     * child process makes status information available by terminating.
     *
     * "pid" - child's process' process ID
     *
     * "status-ptr" points to an object storing the status information 
     * from the child process (unless status-ptr is a null pointer).
     *
     * "options" argument is a bit mask. WNOHANG flag indicates that 
     * the parent process shouldn’t wait.
     *
     * The return value is normally the process ID of the child 
     * process whose status is reported. However, WNOHANG option 
     * means waitpid will return zero instead of blocking. This lets
     * checking if the process got suspended.
     */
    while (waitpid(job->pgid, &termination_status, WNOHANG) == 0) {
        if(job->status == SUSPENDED) {
            return;
        }
    }
    /* delete the job after it is finished */
    jobs_list = del_job(job);
}

void 
kill_job(int job_id) 
{
    if(jobs_list != NULL) {
        JobsList *job = get_job(job_id, 0);
        if (job != NULL) {
            kill(job->pgid, SIGTERM);
        } else {
            printf("kill: %%%d: no such job\n", job_id);
        }
    }   
}

void 
put_job_foreground(JobsList *job, int continue_job, pid_t ppgid) 
{
    if(job == NULL) {
        return;
    }
    tcsetpgrp(STDIN_FILENO, job->pgid);
    if(continue_job) {
        if(kill(job->pgid, SIGCONT) < 0) {
            perror("kill (SIGCONT)");
        }
        printf("\n[%d]+  continued    %s\n", job->id, job->name);
    } else {
        if (job->status == BACKGROUND) {
            printf("\n[%d]+  running    %s\n", job->id, job->name);
        }
    }
    job->status = FOREGROUND;

    wait_job(job);
    tcsetpgrp(STDIN_FILENO, ppgid);
}

void 
put_job_background(JobsList *job, int continue_job, pid_t ppgid)
{
    if(job == NULL) {
        return;
    }
    job->status = BACKGROUND;
    if(continue_job) {
        if(kill(job->pgid, SIGCONT) < 0) {
            perror("kill (SIGCONT)");
        }
        printf("\n[%d]+  continued    %s\n", job->id, job->name);
    }
    tcsetpgrp(STDIN_FILENO, ppgid);
}

void 
print_jobs() 
{
    JobsList *job = jobs_list;
    if (job == NULL) {
        return;
    }

    printf("________________________________________________________________\n");
    printf("| ID   | PID    | STATUS     | STATE | NAME OF EXEC            |\n");
    printf("----------------------------------------------------------------\n");
    while (job != NULL) {
        printf("| [%d]    %-7d  %-11s  %-6s  %-24s|\n", 
                job->id, 
                job->pgid, 
                statuses[job->status][0], 
                statuses[job->status][1], 
                job->name);

        job = job->next;
     }
    printf("----------------------------------------------------------------\n");
}

