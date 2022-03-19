all: pzip.cpp
	g++ pzip.cpp -o pzip -Wall -Werror -pthread -O