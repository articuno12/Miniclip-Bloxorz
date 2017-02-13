all: sample2D

sample2D: mine_game.cpp glad.c
	g++ -g -o game mine_game.cpp glad.c -lGL -lglfw -ldl -lao -lpthread -lSOIL -I/usr/include -I/usr/local/include -I/usr/local/include/freetype2  -std=c++11

clean:
	rm sample2D
