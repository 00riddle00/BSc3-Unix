#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/*#include <readline/readline.h>*/
#include <sys/wait.h>
#include <unistd.h>

#define CMD_BUFF_SIZE 256

void clear_screen() {
    static int first_time = 1; // clear screen for the first time
    if (first_time) {
        const char* CLEAR_SCREEN_ANSI = " \e[1;1H\e[2J";
        write(STDOUT_FILENO, CLEAR_SCREEN_ANSI, 12);
        first_time = 0;
    }
}

char* read_line(const char *prompt) {
    printf("%s", prompt); // disp prompt
    char* line = malloc(CMD_BUFF_SIZE * sizeof(char));
    int count = 0;

    // Read one line
    for (;;) {
        int c = fgetc(stdin);
        if (c == '\n') {
            line[count] = '\0';
            break;
        }
        line[count++] = (char) c;
    }

    return line;
}

char **get_input(char *input) {
    char **command = malloc(CMD_BUFF_SIZE * sizeof(char *));
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
        
int main() {
    char **command;
    char *input;
    pid_t child_pid;
    int stat_loc;

    while (1) {   
        clear_screen();
        input = read_line("{~tsh~}> ");
        if (input[0] == '\0') {
            continue;
        }
        command = get_input(input);

        child_pid = fork();
        if (child_pid == 0) {
            /* Never returns if the call is successful */
            execvp(command[0], command);
        } else {
            waitpid(child_pid, &stat_loc, WUNTRACED);
        }

        free(input);
        free(command);

    }

    return 0;
}
