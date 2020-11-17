#ifndef SIGNALS_H
#define SIGNALS_H

#include <setjmp.h>

/* variables */
extern sigjmp_buf env;

/* ============================= sig_atomic_t =============================
"sig_atomic_t" is only async-signal safe (not thread-safe)

sig_atomic_t is not an atomic data type. It is just the data type 
that you are allowed to use in the context of a signal handler.

Better read the name as "atomic relative to signal handling".

To guarantee communication with and from a signal handler, only 
one of the properties of atomic data types is needed, namely the 
fact that read and update will always see a consistent value,
e.g. sig_atomic_t is guaranteed to be read and written in one go.

So a platform may choose any integer base type as sig_atomic_t for 
which it can make the guarantee that volatile sig_atomic_t can be 
safely used in signal handlers. Many platforms chose int for this, 
because they know that for them int is written with a single instruction.

=============================== volatile ==================================
"volatile" here is essential, since the flag "jump_active" will be accessed 
asynchronously by multiple threads of the process, i.e. the main thread 
and the signal handler thread. The type guarantees atomic access to the 
variable across multiple threads.
*/
extern volatile sig_atomic_t jump_active;

/* function declarations */

/* trap SIGINT and perform long jump */
void sigint_handler();

/* trap SIGCHLD and perform job management */
void sigchld_handler();

#endif /* SIGNALS */
