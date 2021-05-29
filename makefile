build: main.c
	gcc -std=gnu99 -Wall -Werror -g ./main.c -o ./out -I ./ZDK/ -L ./ZDK/ -lzdk -lncurses -lm

clean:
	rm -f ./out

all:
	make clean && make build

run:
	make build && ./out
