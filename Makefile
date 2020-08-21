include config.mk

SRC = doctxt.c util.c
OBJ = ${SRC:.c=.o}

all: options doctxt

options:
	@echo doctxt build options:
	@echo "CFLAGS	= ${CFLAGS}"
	@echo "LDFLAGS	= ${LDFLAGS}"
	@echo "CC		= ${CC}"

${OBJ}: config.mk

.c.o:
	@echo CC $<
	@${CC} -c ${CFLAGS} $<

doctxt: ${OBJ}
	@echo CC -o $@
	@${CC} -o $@ ${OBJ} ${LDFLAGS}

clean:
	@echo cleaning
	@rm -f doctxt ${OBJ} doctxt-${VERSION}.tar.gz

dist: clean
	@echo creating dist tarball
	@mkdir -p doctxt-${VERSION}
	@cp -R LICENSE Makefile config.mk ${SRC} doctxt-${VERSION}
	@tar -cf doctxt-${VERSION}.tar doctxt-${VERSION}
	@gzip doctxt-${VERSION}.tar
	@rm -rf doctxt-${VERSION}

install: all
	@echo installing executable file to ${DESTDIR}${PREFIX}/bin
	@mkdir -p ${DESTDIR}${PREFIX}/bin
	@cp -f doctxt ${DESTDIR}${PREFIX}/bin
	@chmod 755 ${DESTDIR}${PREFIX}/bin/doctxt

uninstall:
	@echo removing executable file from ${DESTDIR}${PREFIX}/bin
	@rm -f ${DESTDIR}${PREFIX}/bin/doctxt


.PHONY: all options clean install uninstall
