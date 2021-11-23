CC = g++
FLAGS = -O2 -std=c++17

cf: experiment.cpp cuckooFilter.h BloomFilter.h parameters.h common.h AwareHash.h
	$(CC) $(FLAGS) experiment.cpp -o cf

clean:
	rm cf