#ifndef JOBS_H
#define JOBS_H

/* macros */
#define FOREGROUND 0
#define BACKGROUND 1
#define SUSPENDED  2

/* structs */
typedef struct Job {
    int id;
    char *name;
    pid_t pid;
    pid_t pgid;
    int status;
    struct Job *next;
} JobsList; /* Linked list */

/* variables */
extern int active_jobs;     /* Number of active jobs (including suspended ones) */
extern JobsList *jobs_list; /* Linked List containing all jobs */

static const char statuses[][2][16] = {
    [FOREGROUND] = { "running",   "S"},
    [BACKGROUND] = { "running",   "S"},
    [SUSPENDED]  = { "suspended", "T"},
};

/* function declarations */

/* add a new job to job's Linked List */
JobsList *add_job(pid_t pgid, char *name, int status);

/* remove a job from the List */
JobsList *del_job(JobsList *job);

/* get certain job's info (struct Job) by job id. 
 * If "search_parameter" = 1, get a job by its pgid. */
JobsList *get_job(int key, int search_parameter);

/* changes the status of the job 
 * (foreground/background/suspended) */
int change_job_status(int pid, int status);

/* this function is used 
 * only with a foreground job
 *
 * it waits for a foreground job to finish, 
 * and then removes the job, unless it was
 * suspended before finishing.
 */
void wait_job(JobsList *job);

/* terminate a job (by sending 
 * SIGTERM using "kill" command) */
void kill_job(int job_id);

/* invoked by parent process whose group id is sent
 * to the function with "ppgid" argument.
 *
 * sets job's status to FOREGROUND, gives it access to 
 * the terminal, and then waits for job to finish by 
 * invoking "wait_job" function (see its description).
 * Afterwards, parent process regains the control of 
 * the terminal.
 *
 * if "continue_job" = 1, SIGCONT signal is sent to 
 * the job.
 */
void put_job_foreground(JobsList *job, int continue_job, pid_t ppgid);

/* invoked by parent process whose group id is sent to 
 * the function with "ppgid" argument.
 *
 * sets job's status to BACKGROUND, and parent process
 * takes control of the terminal.
 *
 * if "continue_job" = 1, SIGCONT signal is sent to 
 * the job.
 */
void put_job_background(JobsList *job, int continue_job, pid_t ppgid);

/* print a table containing the list of active 
 * jobs together with their id, pid (which is 
 * equal to pgid), status, state, and name. */
void print_jobs();

void signal_handler_child();

#endif /* JOBS */
