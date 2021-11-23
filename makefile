CC = g++
FLAGS = -O2 -std=c++17

nze: experiment.cpp sketch.h BloomFilter.h HashTable.h CMsketch.h parameters.h common.h AwareHash.h
	$(CC) $(FLAGS) experiment.cpp -o nze

clean:
	rm nze