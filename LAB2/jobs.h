/* macros */
#define FOREGROUND 'F'
#define BACKGROUND 'B'
#define SUSPENDED  'S'

/* structs */
typedef struct Job {
    int id;
    char *name;
    pid_t pid;
    pid_t pgid;
    int status;
    struct Job *next;
} JobsList; /* Linked list */

/* function declarations */
JobsList *add_job(pid_t pgid, char *name, int status);
JobsList *del_job(JobsList *job);
JobsList *get_job(int key, int search_parameter);
int change_job_status(int pid, int status);
void wait_job(JobsList *job);
void kill_job(int job_id);
void put_job_foreground(JobsList *job, int continue_job, pid_t group_id);
void put_job_background(JobsList *job, int continue_job, pid_t group_id);
void signal_handler_child();
void print_jobs();

/* variables */
extern int active_jobs;
extern JobsList *jobs_list;
