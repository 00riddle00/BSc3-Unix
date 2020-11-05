# tsh - minimal Unix Shell for learning purposes
# See LICENSE file for copyright and license details.

include config.mk

SRC = main.c
OBJ = ${SRC:.c=.o}

all: options tsh

options:
	@echo tsh build options:
	@echo "CFLAGS   = ${CFLAGS}"
	@echo "LDFLAGS  = ${LDFLAGS}"
	@echo "CC       = ${CC}"

.c.o:
	${CC} -c ${CFLAGS} $<

${OBJ}: config.mk

tsh: ${OBJ}
	${CC} -o $@ ${OBJ} ${LDFLAGS}

clean:
	rm -f tsh ${OBJ}
