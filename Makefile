include config.mk

all: build/revelio

clean:
	rm -rf build

install: all
	mkdir -p ${BINPATH} ${MAN1PATH}
	sed 's/MANDATE/${VERSION}/' man/revelio.1 > ${MAN1PATH}/revelio.1
	cp build/revelio ${BINPATH}

build/revelio: src/revelio.c
	mkdir -p build
	${CC} ${CFLAGS} -o${@} ${^}

uninstall:
	rm -f ${BINPATH}/revelio ${MAN1PATH}/revelio.1

.PHONY: all clean install uninstall
