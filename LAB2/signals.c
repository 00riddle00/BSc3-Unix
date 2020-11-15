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
sigchld_handler()
{
    pid_t pid;
    int termination_status;

    /* "pid_t waitpid (pid_t pid, int *status-ptr, int options)"
     *
     * this function requests status information from a child process.
     *
     * "pid" - child's process' process ID. A value of -1 or WAIT_ANY 
     * requests status information for any child process; 
     * 
     * "status-ptr" points to an object storing the status information 
     * from the child process (unless status-ptr is a null pointer).
     *
     * "options" argument is a bit mask. WNOHANG flag indicates that 
     * the parent process shouldn’t wait.
     *
     *     WNOHANG flag specifies that waitpid should return immediately 
     *     instead of waiting, if there is no child process ready to be noticed.
     *
     *     WUNTRACED flag specifies that waitpid should report the status of 
     *     any child processes that have been stopped as well as those that 
     *     have terminated.
     *
     * The return value is the process ID of the child process whose 
     * status is reported. 
     */
    pid = waitpid(WAIT_ANY, &termination_status, WUNTRACED | WNOHANG);

    if (pid > 0) { /* if waitpid() returned process id */
        JobsList *job = get_job(pid, 1);

        if (job == NULL) { /* if the job with such pid does not exist*/
            return;        /* in jobs list, do nothing */
        }

        /* WIFEXITED process completion macro returns a nonzero value 
         * if the child process terminated normally with exit or _exit */
        if (WIFEXITED(termination_status)) {
            /* if the job was in the background, and it terminated
             * without error, notify about its termination. If the
             * error occurred, do not print anything (error handling
             * is done in the child process then) 
             * Remove the job from the List either way
             */
            if (job->status == BACKGROUND) {

                /* If the value of WIFEXITED(stat_val) is non-zero, this 
                 * macro evaluates to the low-order 8 bits of the status 
                 * argument that the child process passed to _exit() or 
                 * exit(), or the value the child process returned from main().
                 */
                int exit_status = WEXITSTATUS(termination_status);

                if(exit_status == 0) {
                    printf("\n[%d]+  done    %s\n", job->id, job->name);
                } else {
                    printf("\n[%d]+  exit %d   %s\n", job->id, exit_status, job->name);
                }
                jobs_list = del_job(job);
            }
        }
        /* WIFSIGNALED process completion macro returns a nonzero value 
         * if the child process terminated because it received a signal 
         * that was not handled. In this program's case, it's SIGTERM, 
         * sent with "kill" command by a user */
        else if (WIFSIGNALED(termination_status)) {
            printf("\n[%d]+  terminated    %s\n", job->id, job->name);
            jobs_list = del_job(job);
        }
        /* WIFSTOPPED process completion macro returns a nonzero value 
         * if the child process is stopped.
         * In this program's case, it's SIGTSTP sent with ^Z by a user */
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
