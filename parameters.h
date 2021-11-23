#ifndef _PARAMETERS_H
#define _PARAMETERS_H

//parameters of Cuckoo Filter
#define MAX_TRY 50
#define BUCKET_NUM 4

//parameters of Bloom Filter
#define BLOOM_SIZE (1 << 20)
#define BLOOM_HASH_NUM 11

//parameters of hash table
#define SLOT_NUM (1 << 15)
#define EVICT_THRESHOLD 1

//Key length
#define KEY_T_SIZE 13

//return value of hash table's insertion
#define HIT 0
#define MISS_EVICT 1
#define MISS_INSERT 2

#endif