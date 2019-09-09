SRCDIR = .
OBJDIR = .
VPATH  = ${shell for dir in `find ${SRCDIR} -type d`;do echo -n $$dir:;done}

CC = gcc
CFLAGS = -Iobject -I./ -I/usr/include/glib-2.0 -I/usr/lib64/glib-2.0/include  -lglib-2.0
TARGET = main
CSOURCES = ${shell find  ${SRCDIR} -name \*.c}
OBJECTS = ${shell for obj in ${CSOURCES:.c=.o}; do echo ${OBJDIR}/`basename $$obj`;done}

${OBJDIR}/%.o: %.c
	@${CC} -c ${CFLAGS} ${CPPFLAGS} $< -o $@

${TARGET}: ${OBJECTS} 
	@${CC}  ${CFLAGS} ${LDFLAGS} ${OBJECTS} -o $@

clean:
	@rm -f *.o ${TARGET}

