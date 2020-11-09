#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#ifdef READLINE
#include <readline/readline.h>
#endif /* READLINE */
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <setjmp.h>
#include <time.h>
/* useful debugging macros */
#include "dbg.h"

#ifndef CMD_BUFF_SIZE
#define CMD_BUFF_SIZE 512
#endif

/* enums */
enum { StylePrompt, StyleErrPrefix, StyleErrMsg, StyleErrInput }; /* style */
enum { fg, bg, bold, uline, blink };                              /* style */

/* configuration, allows nested code to access above variables */
#include "config.h"

char *read_line(const char *);
char **get_input(char *);
void clear_screen(int);
int  cd(char *);
void sigint_handler();
char *set_style(int, int, int, int, int);
char *reset_style();
char *str_replace(char *, char *, char *);

static sigjmp_buf env;
static volatile sig_atomic_t jump_active = 0;


int main() {
    char **command;
    char *input;
    pid_t child_pid;
    int stat_loc;
    int startup_curr_cmd = 0;
    /*int job_count = 0;*/

    errno = 0;

    /* Setup info to be displayed at the prompt */
    char *user_name = getenv("USER");

    // TODO get rid of magic numbers
    char home_dir[20];
    sprintf(home_dir, "/home/%s", user_name);

    char host_name[20];
    gethostname(host_name, 20);

    char current_dir[512];
    getcwd(current_dir, 512);

    time_t curr_time;
    char *time_str;
    struct tm* time_info;

    char date_buffer[10];
    char time_buffer[5];

    /* Setup SIGINT */
    struct sigaction s;
    s.sa_handler = sigint_handler;
    sigemptyset(&s.sa_mask);
    s.sa_flags = SA_RESTART;
    sigaction(SIGINT, &s, NULL);

    while (1) {   
        if (sigsetjmp(env, 1) == 42) {
            printf("\n");
            continue;
        }

        jump_active = 1;

        clear_screen(0);

        if (startup_curr_cmd != startup_cmd_count) {
            input = strdup(startup[startup_curr_cmd]);
            startup_curr_cmd++;
        } else {
            curr_time = time(NULL);
            time_str = ctime(&curr_time);
            time_str[strlen(time_str)-1] = '\0';
            time_info = localtime(&curr_time);

            strftime(date_buffer, 10, "%Y-%m-%d", time_info);
            strftime(time_buffer, 5, "%H:%M", time_info);

            // TODO optimize: check all the flags beforehand
            input = read_line(
                str_replace(
                    str_replace(
                        str_replace(
                            str_replace(
                                str_replace(
                                    str_replace(
                                        prompt,
                                        "%u",
                                        user_name),
                                    home_dir,
                                    "~"),
                                "%h",
                                host_name),
                            "%w", 
                            getcwd(current_dir, 512) ),
                        "%d", date_buffer),
                    "%t", time_buffer) );
        }

        if (input == NULL) { /* Exit on Ctrl-D */
            printf("exit\n");
            exit(0);
        }

        /* Handle empty input */
        if (input[0] == '\0') {
            continue;
        }

        command = get_input(input);

        if (strcmp(command[0], "cd") == 0) {
            if (cd(command[1]) < 0) {
                perror(command[1]);
            }

            // Skip the fork
            continue;
        }

        if (strcmp(command[0], "exit") == 0 ||
            strcmp(command[0], alias_exit) == 0) {

            exit(0);
        }

        if (strcmp(command[0], alias_cls) == 0) {
            clear_screen(1);

            // Skip the fork
            continue;
        }

        child_pid = fork();
        if (child_pid <0) {
            perror("Fork failed");
            exit(1);
        }

        if (child_pid == 0) {
            struct sigaction s;
            s.sa_handler = sigint_handler;
            sigemptyset(&s.sa_mask);
            s.sa_flags = SA_RESTART;
            sigaction(SIGINT, &s, NULL);

            /* Never returns if the call is successful */
            if (execvp(command[0], command) < 0) {

                // TODO print "command not found" instead
                // of "no such file or directory"
                fprintf(stderr,
                        "%stsh: %s%s%s: %s%s%s%s\n", 
                        set_style( style[StyleErrPrefix][fg],
                                   style[StyleErrPrefix][bg],
                                   style[StyleErrPrefix][bold],
                                   style[StyleErrPrefix][uline],
                                   style[StyleErrPrefix][blink] ),
                        reset_style(),
                        set_style( style[StyleErrMsg][fg],
                                   style[StyleErrMsg][bg],
                                   style[StyleErrMsg][bold],
                                   style[StyleErrMsg][uline],
                                   style[StyleErrMsg][blink] ),
                        strerror(errno),
                        reset_style(),
                        set_style( style[StyleErrInput][fg],
                                   style[StyleErrInput][bg],
                                   style[StyleErrInput][bold],
                                   style[StyleErrInput][uline],
                                   style[StyleErrInput][blink] ),
                        command[0],
                        reset_style() );

                exit(1);
            }
        } else {
            waitpid(child_pid, &stat_loc, WUNTRACED);
        }

        if (!input)
            free(input);
        if (!command)
            free(command);
    }

    return 0;
}

#ifdef READLINE
char *read_line(const char *prompt) {
    return readline(prompt);
}
#else
char *read_line(const char *prompt) {

    printf("%s%s%s", 
            set_style( style[StylePrompt][fg],
                       style[StylePrompt][bg],
                       style[StylePrompt][bold],
                       style[StylePrompt][uline],
                       style[StylePrompt][blink] ),
            prompt,
            reset_style() );

    char *line = malloc(CMD_BUFF_SIZE * sizeof(char));
    if (line == NULL) {
        perror("malloc failed");
        exit(1);
    }

    int count = 0;
    int blank_line = 1;

    // Read one line
    for (;;) {
        int c = fgetc(stdin);

        if (c == EOF) {
            if (!count) {
                return (char *) NULL;
            } else {
                continue;
            }
        }

        if (c == '\n') {
            if (blank_line) {
                line[0] = '\0';
                break;
            } else {
                line[count] = '\0';
                break;
            }
        }

        if (c != ' ') {
            blank_line = 0;
        }

        line[count++] = (char) c;
    }

    return line;
}
#endif /* READLINE */

char **get_input(char *input) {
    char **command = malloc(CMD_BUFF_SIZE * sizeof(char *));
    if (command == NULL) {
        perror("malloc failed");
        exit(1);
    }

    char *separator = " ";
    char *parsed;
    int index = 0;

    parsed = strtok(input, separator);
    while (parsed != NULL) {
        command[index++] = parsed;
        parsed = strtok(NULL, separator);
    }

    command[index] = NULL;
    return command;
}

void clear_screen(int do_it) {
    if (do_it < 0 || do_it > 1) {
        printf("invalid argument to clear_screen()");
        exit(1);
    }

    const char *CLEAR_SCREEN_ANSI = " \x1b[1;1H\x1b[2J";
    static int first_time = 1; // clear screen for the first time

    if (first_time) {
        first_time = 0;
        write(STDOUT_FILENO, CLEAR_SCREEN_ANSI, 12);
    } else if (do_it) {
        write(STDOUT_FILENO, CLEAR_SCREEN_ANSI, 12);
    }
}

int cd(char *path) {
    return chdir(path);
}

void sigint_handler() {
    if (!jump_active) {
        return;
    }
    // TODO avoid magic numbers
    siglongjmp(env, 42);
}

char *set_style(int fg, int bg, int bold, int uline, int blink) {

    fg    = (fg >= 0 && fg <= 255) ? fg   : -1;  /* ANSI: ESC[38;5;{0-255}m -> foreground */
    bg    = (bg >= 0 && bg <= 255) ? bg   : -1;  /* ANSI: ESC[48;5;{0-255}m -> background */
    bold  = (bold  == 1)           ? 1    : -1;  /* ANSI: ESC[1m            -> bold       */
    uline = (uline == 1)           ? 4    : -1;  /* ANSI: ESC[4m            -> underline  */
    blink = (blink == 1)           ? 5    : -1;  /* ANSI: ESC[5             -> blink      */

    // TODO avoid magic numbers
    char *style_str = malloc(sizeof(char) * 64);

    sprintf( style_str,
             "\x1b[38;5;%dm"\
             "\x1b[48;5;%dm"\
             "\x1b[%dm"\
             "\x1b[%dm"\
             "\x1b[%dm",
             fg,
             bg,
             bold,
             uline,
             blink );

    return style_str;
}

char *reset_style() {
    return "\x1b[0m";
}

// You must free the result if result is non-NULL.
char *str_replace(char *orig, char *rep, char *with) {
    char *result; // the return string
    char *ins;    // the next insert point
    char *tmp;    // varies
    int len_rep;  // length of rep (the string to remove)
    int len_with; // length of with (the string to replace rep with)
    int len_front; // distance between rep and end of last rep
    int count;    // number of replacements

    // sanity checks and initialization
    if (!orig || !rep)
        return NULL;
    len_rep = strlen(rep);
    if (len_rep == 0)
        return NULL; // empty rep causes infinite loop during count
    if (!with)
        with = "";
    len_with = strlen(with);

    // count the number of replacements needed
    ins = orig;
    for (count = 0; (tmp = strstr(ins, rep)); ++count) {
        ins = tmp + len_rep;
    }

    tmp = result = malloc(strlen(orig) + (len_with - len_rep) * count + 1);

    if (!result)
        return NULL;

    // first time through the loop, all the variable are set correctly
    // from here on,
    //    tmp points to the end of the result string
    //    ins points to the next occurrence of rep in orig
    //    orig points to the remainder of orig after "end of rep"
    while (count--) {
        ins = strstr(orig, rep);
        len_front = ins - orig;
        tmp = strncpy(tmp, orig, len_front) + len_front;
        tmp = strcpy(tmp, with) + len_with;
        orig += len_front + len_rep; // move to next "end of rep"
    }
    strcpy(tmp, orig);
    return result;
}
