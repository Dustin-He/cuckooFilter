#ifndef _PARAMETERS_H
#define _PARAMETERS_H

//parameters of Cuckoo Filter
#define BYTE2BIT 8
#define INT64BIT 64
#define INT64BYTE 8
#define MAX_TRY 1000
#define BUCKET_NUM 4
#define FINGERPRINTNUM (1 << 15)
#define FINGERPRINTBITSIZE 24

//parameters of Bloom Filter
#define BLOOM_SIZE (1 << 19)
#define BLOOM_HASH_NUM 7

//Key length
#define KEY_T_SIZE 13

//return value of Cuckoo Filter's function
#define NOT_EMPTY 0
#define IS_EMPTY 1
#define EXIST 2

#endif