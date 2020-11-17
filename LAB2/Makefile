# tsh - minimal Unix Shell for learning purposes
# See LICENSE file for copyright and license details.

include config.mk

SRC = main.c jobs.c signals.c utils.c
OBJ = ${SRC:.c=.o}

all: options tsh

options:
	@echo tsh build options:
	@echo "CFLAGS   = ${CFLAGS}"
	@echo "LDFLAGS  = ${LDFLAGS}"
	@echo "CC       = ${CC}"

# "$<" - the name of the first prerequisite. If the target got its 
# recipe from an implicit rule, this will be the first prerequisite 
# added by the implicit rule (see Implicit Rules).
.c.o:
	${CC} -g -c ${CFLAGS} $<

main.o jobs.o signals.o utils.o: config.mk
#${OBJ}: config.mk

#tsh: main.o jobs.o signals.o utils.o
#	#gcc -o tsh main.o jobs.o signals.o utils.o
tsh: ${OBJ}
	${CC} -o $@ ${OBJ} ${LDFLAGS}

#clean:
#	rm -f tsh main.o jobs.o signals.o utils.o
clean:
	rm -f tsh ${OBJ}

dist: clean
	$(info ************ MAKING AN ARCHIVE ***********)
	mkdir -p tsh-${VERSION}
	cp -R LICENSE Makefile README.md config.mk\
		tsh.1 jobs.h signals.h utils.h ${SRC} tsh.png prompt.gif tsh-${VERSION}
	tar -cf tsh-${VERSION}.tar tsh-${VERSION}
	gzip tsh-${VERSION}.tar
	rm -rf tsh-${VERSION}
	@echo \********************DONE******************

install: all
	$(info **************** INSTALLING TSH ON YOUR SYSTEM ***************)
	mkdir -p ${DESTDIR}${PREFIX}/bin
	cp -f tsh ${DESTDIR}${PREFIX}/bin
	chmod 755 ${DESTDIR}${PREFIX}/bin/tsh
	mkdir -p ${DESTDIR}${MANPREFIX}/man1
	sed "s/VERSION/${VERSION}/g" < tsh.1 > ${DESTDIR}${MANPREFIX}/man1/tsh.1
	chmod 644 ${DESTDIR}${MANPREFIX}/man1/tsh.1
	@echo \******************************DONE*****************************

uninstall:
	$(info **************** REMOVING TSH FROM YOUR SYSTEM ****************)
	rm -f ${DESTDIR}${PREFIX}/bin/tsh\
		${DESTDIR}${MANPREFIX}/man1/tsh.1
	@echo \******************************DONE*****************************

.PHONY: all options clean dist install uninstall
