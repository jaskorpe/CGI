CC=cc

CFLAGS=-Wall -Wextra -c

all: brainfuck.cgi

brainfuck.cgi: brainfuck.o brainfuck_interpreter.o
	$(CC) -o $@ $^ -lcgi

brainfuck.o:brainfuck_cgi.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f brainfuck.cgi *.o