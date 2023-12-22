include config.mk

all: revelio

clean:
	rm -f revelio

install: all
	mkdir -p ${BINPATH} ${MAN1PATH}
	mv revelio ${BINPATH}
	sed 's/MANDATE/${VERSION}/' revelio.1 > ${MAN1PATH}/revelio.1

revelio: revelio.c
	${CC} ${CFLAGS} -o${@} ${^}

uninstall:
	rm -f ${BINPATH}/revelio ${MAN1PATH}/revelio.1

.PHONY: all clean install uninstall
