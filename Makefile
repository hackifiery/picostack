all: ps
ps:
	gcc src/*.c -o picostack -Wall -Wextra -std=c99 -pedantic -Wmissing-prototypes -Wstrict-prototypes -Wshadow -Wpointer-arith -Wcast-qual -Wwrite-strings -Wconversion -Wsign-conversion -Wunreachable-code -Wfloat-equal -Wundef -Wbad-function-cast -Winline -Wnested-externs
clean:
	rm -f picostack
