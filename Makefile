
all: main/main.cpp
	g++ -o fileserver main/main.cpp

clean:
	rm -rf fileserver
