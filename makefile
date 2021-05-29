exe_name = out
CFLAGS = -lzdk -lncurses -lm -std=gnu99 -Wall -Werror -g
CLIBS = -I ./ZDK/ -L ./ZDK/
DEPS = state.h
all: clean state.o main.o build

run: build
	./$(exe_name)

build: main.o state.o
	gcc ./main.o state.o -o $(exe_name) $(CLIBS) $(CFLAGS)

main.o: main.c state.h
	gcc -c main.c $(CLIBS) $(CFLAGS)

state.o: state.c state.h
	gcc -c state.c $(CLIBS) $(CFLAGS)

clean:
	rm -f $(exe_name)