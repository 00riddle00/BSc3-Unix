# tsh - minimal Unix Shell for learning purposes
# See LICENSE file for copyright and license details.

include config.mk

SRC = tsh.c
OBJ = ${SRC:.c=.o}

all: options tsh

options:
	@echo tsh build options:
	@echo "CFLAGS   = ${CFLAGS}"
	@echo "LDFLAGS  = ${LDFLAGS}"
	@echo "CC       = ${CC}"

.c.o:
	${CC} -g -c ${CFLAGS} $<

${OBJ}: config.mk

tsh: ${OBJ}
	${CC} -o $@ ${OBJ} ${LDFLAGS}

clean:
	rm -f tsh ${OBJ}

dist: clean
	mkdir -p tsh-${VERSION}
	cp -R LICENSE Makefile README.md config.mk ${SRC} tsh.png tsh-${VERSION}
	tar -cf tsh-${VERSION}.tar tsh-${VERSION}
	gzip tsh-${VERSION}.tar
	rm -rf tsh-${VERSION}

install: all
	mkdir -p ${DESTDIR}${PREFIX}/bin
	cp -f tsh ${DESTDIR}${PREFIX}/bin
	chmod 755 ${DESTDIR}${PREFIX}/bin/tsh
	mkdir -p ${DESTDIR}${MANPREFIX}/man1
	sed "s/VERSION/${VERSION}/g" < tsh.1 > ${DESTDIR}${MANPREFIX}/man1/tsh.1
	chmod 644 ${DESTDIR}${MANPREFIX}/man1/tsh.1

uninstall:
	rm -f ${DESTDIR}${PREFIX}/bin/tsh\
		${DESTDIR}${MANPREFIX}/man1/tsh.1

.PHONY: all options clean dist install uninstall
