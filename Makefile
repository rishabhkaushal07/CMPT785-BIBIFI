CFLAGS = -Wall -Werror -std=c++17

all: main/main.cpp
	g++ $(CFLAGS) -o fileserver main/main.cpp

clean:
	rm -rf fileserver
