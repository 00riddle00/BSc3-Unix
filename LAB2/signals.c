/* function implementations */

#include <setjmp.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

#include "jobs.h"
#include "signals.h"

/* variables */
sigjmp_buf env;
volatile sig_atomic_t jump_active = 0;

/* function implementations */
void 
sigint_handler() 
{
    if (!jump_active) {
        return;
    }
    // TODO avoid magic numbers
    siglongjmp(env, 42);
}

void 
signal_handler_child() 
{
    pid_t pid;
    int termination_status;
    pid = waitpid(WAIT_ANY, &termination_status, WUNTRACED | WNOHANG);
    if (pid > 0) {
        JobsList *job = get_job(pid, 1);
        if (job == NULL) {
            return;
        }

        if (WIFEXITED(termination_status)) {
            if (job->status == BACKGROUND) {
                printf("\n[%d]+  done    %s\n", job->id, job->name);
                jobs_list = del_job(job);
            }
        }
        else if (WIFSIGNALED(termination_status)) {
            printf("\n[%d]+  terminated    %s\n", job->id, job->name);
            jobs_list = del_job(job);
        }
        else if (WIFSTOPPED(termination_status)) {
            tcsetpgrp(STDIN_FILENO, job->pgid);
            change_job_status(pid, SUSPENDED);
            printf("\n[%d]+   suspended    %s\n", active_jobs, job->name);
            return;
        } else {
            if (job->status == BACKGROUND) {
                jobs_list = del_job(job);
            }
        }
    }
}
