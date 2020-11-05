#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
/*#include <readline/readline.h>*/
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <setjmp.h>

#define CMD_BUFF_SIZE 256

#define ANSI_COLOR_RED         "\x1b[0;31m"
#define ANSI_COLOR_BLUE        "\x1b[0;34m"
#define ANSI_COLOR_CYAN        "\x1b[0;36m"
#define ANSI_COLOR_WHITE       "\x1b[0;37m"

#define ANSI_COLOR_RED_BOLD    "\x1b[1;31m"
#define ANSI_COLOR_BLUE_BOLD   "\x1b[1;34m"
#define ANSI_COLOR_CYAN_BOLD   "\x1b[1;36m"
#define ANSI_COLOR_WHITE_BOLD  "\x1b[1;37m"

#define ANSI_COLOR_RESET       "\x1b[0m"

char *read_line(const char *);
char **get_input(char *);
void clear_screen();
int  cd(char *);
void sigint_handler(int);

static sigjmp_buf env;
static volatile sig_atomic_t jump_active = 0;

int main() {
    char **command;
    char *input;
    pid_t child_pid;
    int stat_loc;

    errno = 0;

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

        clear_screen();
        input = read_line("{~tsh~}> ");

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

        if (strcmp(command[0], "exit") == 0) {
            exit(0);
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
                fprintf(stderr, ANSI_COLOR_CYAN_BOLD "tsh: " ANSI_COLOR_RED_BOLD "%s: " ANSI_COLOR_WHITE "%s\n" ANSI_COLOR_RESET, strerror(errno), command[0]);
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

char *read_line(const char *prompt) {
    printf(ANSI_COLOR_BLUE_BOLD "%s" ANSI_COLOR_RESET, prompt); // disp prompt
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

void clear_screen() {
    static int first_time = 1; // clear screen for the first time
    if (first_time) {
        const char *CLEAR_SCREEN_ANSI = " \e[1;1H\e[2J";
        write(STDOUT_FILENO, CLEAR_SCREEN_ANSI, 12);
        first_time = 0;
    }
}

int cd(char *path) {
    return chdir(path);
}

void sigint_handler(int signo) {
    if (!jump_active) {
        return;
    }
    siglongjmp(env, 42);
}
