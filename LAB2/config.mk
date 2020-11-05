# tsh version
VERSION = 0.1

# Customize below to fit your system

# paths
PREFIX = /usr/local
MANPREFIX = ${PREFIX}/share/man

# Readline, comment if you don't want it
#READLINELIB  = -lreadline
#READLINEFLAG = -DREADLINE

# includes and libs
LIBS = ${READLINELIB}

# flags
#CPPFLAGS = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_POSIX_C_SOURCE=200809L -DVERSION=\"${VERSION}\" ${READLINEFLAG}
CPPFLAGS = -DVERSION=\"${VERSION}\" ${READLINEFLAG}
#CFLAGS   = -std=c99 -pedantic -Wall -Wextra -Wno-deprecated-declarations ${CPPFLAGS}
CFLAGS   = -pedantic -Wall -Wextra -Wno-deprecated-declarations ${CPPFLAGS}
LDFLAGS  = ${LIBS}

# compiler and linker
CC=gcc
