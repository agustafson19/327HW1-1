
all: game

game: game.c
	gcc -Wall -Werror -ggdb3 -o game game.c

clean:
	rm -f game
