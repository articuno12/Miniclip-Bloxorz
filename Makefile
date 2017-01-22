all: sample2D

sample2D: mine_game.cpp glad.c
	g++ -o game mine_game.cpp glad.c -lGL -lglfw -ldl -lao -lpthread -std=c++11

clean:
	rm sample2D
