DEBUG = -g
CFLAGS = ${DEBUG} -Wall

EXEC = tar
OBJS = tar.o

all: ${EXEC}

${EXEC}: ${OBJS}
	g++ ${CFLAGS} -o ${EXEC} ${OBJS}

${OBJS}: %.o: %.cpp
	g++ ${CFLAGS} -c -o $@ $<

clean:
	rm -f ${EXEC} ${OBJS}
