exe_name = out
CFLAGS = -lzdk -lncurses -lm -std=gnu99 -Wall -Werror -g
CLIBS = -I ./ZDK/ -I ./ -L ./ZDK/
DEPS = state.h
all: clean util.o platforms.o zombies.o state.o collision.o main.o build

run: build
	./$(exe_name)

build: main.o state.o platforms.o zombies.o util.o collision.o
	gcc ./main.o state.o platforms.o zombies.o util.o collision.o -o $(exe_name) $(CLIBS) $(CFLAGS)

main.o: main.c state.h platforms.h images.h collision.h util.h
	gcc -c main.c $(CLIBS) $(CFLAGS)

state.o: state.c
	gcc -c state.c $(CLIBS) $(CFLAGS)

zombies.o: zombies.c
	gcc -c zombies.c $(CLIBS) $(CFLAGS)

platforms.o: platforms.c
	gcc -c platforms.c $(CLIBS) $(CFLAGS)

collision.o: collision.c
	gcc -c collision.c $(CLIBS) $(CFLAGS)

util.o: util.c
	gcc -c util.c $(CLIBS) $(CFLAGS)

clean:
	rm -f $(exe_name) && rm -f *.o