#ifndef SIGNALS_H
#define SIGNALS_H

#include <setjmp.h>

/* variables */
extern sigjmp_buf env;
extern volatile sig_atomic_t jump_active;

/* function declarations */

/* trap SIGINT and perform long jump */
void sigint_handler();

/* trap SIGCHLD and perform job management */
void sigchld_handler();

#endif /* SIGNALS */
