#ifndef _CUCKOO_FILTER_H
#define _CUCKOO_FILTER_H

#include "common.h"
// #include "parameters.h"

template<typename T, uint32_t fingerprintNum, uint32_t fingerprintBitSize>
class fingerprintArray {
	uint32_t arraySize, cellSize, cellNum;
	T* builtInArray;

public:
	fingerprintArray() {
		uint32_t bitNum = fingerprintNum * fingerprintBitSize;
		cellSize = sizeof(T);
		cellNum = bitNum / (cellSize * BYTE2BIT) + (bitNum % (cellSize * BYTE2BIT) != 0);
		// cellNum = fingerprintNum;
		arraySize = cellSize * cellNum;
		builtInArray = new T[cellNum]();
	}

	~fingerprintArray() {
		delete []builtInArray;
	}

	uint64_t *getPosition(uint32_t index, uint32_t &startBit) {
		uint32_t startPosition = index * fingerprintBitSize / (cellSize * BYTE2BIT);
		assert(startPosition < cellNum);
		startBit = index * fingerprintBitSize % (cellSize * BYTE2BIT);
		return (uint64_t *)&builtInArray[startPosition];
	}

	uint64_t get(uint32_t index) {
		uint32_t startBit;
		uint64_t *startPointer = getPosition(index, startBit);
		uint64_t mask1, mask2;
		uint64_t ret;

		mask1 = (startBit == 0)?-1:(((uint64_t)1 << (INT64BIT - startBit)) - 1);
		if (startBit + fingerprintBitSize - 1 < INT64BIT) {
			ret = ((*startPointer) & mask1) >> (INT64BIT - startBit - fingerprintBitSize);
		}
		else {
			mask2 = ~(((uint64_t)1 << ( 2 * INT64BIT - (startBit + fingerprintBitSize))) - 1);
			ret = (((*startPointer) & mask1) << (startBit + fingerprintBitSize - INT64BIT)) + (((*(startPointer + 1)) & mask2) >> (2 * INT64BIT - (startBit + fingerprintBitSize)));
			// std::cout << startPointer << " " << startPointer + 1<< std::endl;
		}

		return ret;
		// return builtInArray[index];
	}

	uint64_t *clear(uint32_t index, uint32_t& startBit) {
		uint64_t *startPointer = getPosition(index, startBit);
		uint64_t tmp = (startBit == 0)?-1:(((uint64_t)1 << (INT64BIT - startBit)) - 1);
		uint64_t mask1, mask2;
		uint64_t ret;

		if (startBit + fingerprintBitSize - 1 < INT64BIT) {
			mask1 = ~(tmp - (((uint64_t)1 << (INT64BIT - startBit - fingerprintBitSize)) - 1));
			(*startPointer) &= mask1;
		}
		else {
			mask1 = ~tmp;
			mask2 = (((uint64_t)1 << ( 2 * INT64BIT - (startBit + fingerprintBitSize))) - 1);
			(*startPointer) &= mask1;
			(*(startPointer+1)) &= mask2;
		}

		// builtInArray[index] = 0;

		return startPointer;
		// return &builtInArray[index];
	}

	void assign(uint32_t index, uint64_t fingerprint) {
		uint32_t startBit;
		uint64_t *startPointer = clear(index, startBit);
		uint64_t mask;

		if (startBit + fingerprintBitSize - 1 < INT64BIT) {
			(*startPointer) ^= (fingerprint << (INT64BIT - startBit - fingerprintBitSize));
		}
		else {
			mask = (((uint64_t)1 << (startBit + fingerprintBitSize - INT64BIT)) - 1);
			(*startPointer) ^= fingerprint >> (startBit + fingerprintBitSize - INT64BIT);
			*(startPointer+1) ^= (fingerprint & mask) << (2 * INT64BIT - (startBit + fingerprintBitSize));
		}	

		// builtInArray[index] = fingerprint;	
	}

	size_t getMemoryUsage() {
		return cellNum * cellSize;
	}
};

template<typename T, uint32_t fingerprintNum, uint32_t fingerprintBitSize>
class cuckooFilter {
	fingerprintArray<T, fingerprintNum, fingerprintBitSize> fingerprints[BUCKET_NUM];
	int replacementCounter;
	uint64_t h, s, n, hf, sf, nf;
	uint8_t testkey[KEY_T_SIZE] = {127, 58, 87, 119, 244, 158, 83, 125, 0, 80, 110, 39, 6};

	uint32_t calcPosition(Key_t key) {
		return (uint32_t)(AwareHash((unsigned char *)key, KEY_T_SIZE, h, s, n) % fingerprintNum);
	}

	uint64_t calcFingerprint(Key_t key) {
		return (uint64_t)(AwareHash((unsigned char *)key, KEY_T_SIZE, hf, sf, nf) & (((uint64_t)1 << fingerprintBitSize) - 1));
	}

	uint32_t calcFingerprintHash(uint64_t *fingerprint) {
		uint32_t ans = (uint32_t)(AwareHash((unsigned char *)fingerprint, INT64BYTE, h, s, n) % fingerprintNum);
		return ans;
	}

	int hasEmptyBucket(uint32_t position, int &index, uint64_t fingerprint) {
		uint64_t bucket[BUCKET_NUM];
		for (int i = 0; i < BUCKET_NUM; ++i) {
			bucket[i] = fingerprints[i].get(position);
			if (bucket[i] == fingerprint)
				return EXIST;
			else if (index < 0 && bucket[i] == 0) {
				index = i;
			}
		}

		if (index >= 0)
			return IS_EMPTY;
		return NOT_EMPTY;
	}

	bool compareKey(Key_t key) {
		for (int i = 0; i < KEY_T_SIZE; ++i) {
			if ((uint8_t)key[i] == testkey[i])
				continue;
			else
				return false;
		}
		return true;
	}

public:
	cuckooFilter() {
		int index = 0;
		h = GenHashSeed(index++);
		s = GenHashSeed(index++);
		n = GenHashSeed(index++);
		hf = GenHashSeed(index++);
		sf = GenHashSeed(index++);
		nf = GenHashSeed(index++);
		replacementCounter = 0;
	}

	int insert(Key_t key) {
		uint32_t position1 = calcPosition(key);
		uint64_t fingerprint = calcFingerprint(key), tmpFingerprint;
		int index = -1;
		int ret;

		assert(position1 < fingerprintNum);

		// bool flag = compareKey(key);
		
		while ((ret = hasEmptyBucket(position1, index, fingerprint)) == NOT_EMPTY) {
			tmpFingerprint = fingerprints[0].get(position1);
			fingerprints[0].assign(position1, fingerprint);
			position1 ^= calcFingerprintHash(&tmpFingerprint);
			assert(position1 < fingerprintNum);
			fingerprint = tmpFingerprint;
			replacementCounter++;
			if (replacementCounter > MAX_TRY) {
				std::cout << "Too many replacements!" << std::endl;
				return 1;
			}
		}
		if (ret == IS_EMPTY) {
			// std::cout << "In insert:" << position1  << std::endl;
			fingerprints[index].assign(position1, fingerprint);
		}
		return 0;
	}

	bool query(Key_t key) {
		uint32_t position1 = calcPosition(key);
		uint64_t fingerprint = calcFingerprint(key);
		uint32_t position2 = position1 ^ calcFingerprintHash(&fingerprint);
		int index = -1;
		
		assert(position1 < fingerprintNum);
		assert(position2 < fingerprintNum);

		// if (compareKey(key))
		// std::cout << "In query " << position1 << " " << position2 << std::endl;
		if (hasEmptyBucket(position1, index, fingerprint) == EXIST) {
			return true;
		}

		if (hasEmptyBucket(position2, index, fingerprint) == EXIST) {
			return true;
		}

		return false;
	}

	double getLoadRate() {
		double cnt = 0;
		for (int i = 0; i < BUCKET_NUM; ++i) {
			for (int j = 0; j < fingerprintNum; ++i) {
				if (fingerprints[i].get(j) != 0)
					cnt += 1;
			}
		}
		return cnt / (BUCKET_NUM * fingerprintNum);
	}

	size_t getMemoryUsage() {
		return fingerprints[0].getMemoryUsage() * BUCKET_NUM;
	}
};

#endif