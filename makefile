exe_name = out
CFLAGS = -lzdk -lncurses -lm -std=gnu99 -Wall -Werror -g
CLIBS = -I ./ZDK/ -I ./ -L ./ZDK/
DEPS = state.h
all: clean platforms.o state.o main.o build

run: build
	./$(exe_name)

build: main.o state.o platforms.o util.o
	gcc ./main.o state.o platforms.o util.o -o $(exe_name) $(CLIBS) $(CFLAGS)

main.o: main.c state.h platforms.h images.h
	gcc -c main.c $(CLIBS) $(CFLAGS)

state.o: state.c state.h
	gcc -c state.c $(CLIBS) $(CFLAGS)

platforms.o: platforms.c platforms.h
	gcc -c platforms.c $(CLIBS) $(CFLAGS)

util.o: util.c util.h
	gcc -c util.c $(CLIBS) $(CFLAGS)

clean:
	rm -f $(exe_name)