CC = g++
CFLAGS = -Wall -O3 -DABSTRACT -fno-stack-protector -Wno-write-strings -Wno-deprecated -Wno-parentheses -Wno-missing-field-initializers -Wno-unused-parameter -Wno-uninitialized

.SUFFIXES: .o .C .h

all: main
	
main: main.o
	$(CC) -g main.o -o if2dot -L$(IF)/bin/`${IF}/com/arch` -lmodel
	
main.o :
	${CC} -g -c -o main.o $(CFLAGS) -I${IF}/src/model  main.C

install:
	mv if2dot ~/bin/

clean:
	-rm -f *~ *.o main
