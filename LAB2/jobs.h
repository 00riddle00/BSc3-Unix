
/* macros */
#define MAX_LENGTH 100
#define MAX_PIPES 10
#define MAX_ARGUMENTS 20
#define FOREGROUND 'F'
#define BACKGROUND 'B'
#define SUSPENDED 'S'

/* structs */
typedef struct Job {
    int id;
    char *name;
    pid_t pid;
    pid_t pgid;
    int status;
    struct Job *next;
} JobsList;

/* function declarations */
JobsList *add_job(pid_t , char *, int);
JobsList *del_job(JobsList *);
JobsList *get_job(int, int);
int change_job_status(int, int);
void wait_job(JobsList *);
void kill_job(int);
void put_job_foreground(JobsList *, int);
void put_job_background(JobsList *, int);
void signal_handler_child();
void start_job(char *command[], int);
void print_jobs();

/* variables */
extern int mode;
extern pid_t groupID;

extern int activeJobs;
extern JobsList *jobsList;

