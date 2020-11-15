#ifndef SIGNALS_H
#define SIGNALS_H

#include <setjmp.h>

/* variables */
extern sigjmp_buf env;
extern volatile sig_atomic_t jump_active;

/* function declarations */
void sigint_handler();
void signal_handler_child();

#endif /* SIGNALS */
