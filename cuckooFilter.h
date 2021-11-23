#ifndef _CUCKOO_FILTER_H
#define _CUCKOO_FILTER_H

template<typenanme T, uint32_t fingerprintNum, uint32_t fingerprintBitSize>
class fingerprintArray {
	uint32_t arraySize, cellSize, cellNum;
	T* builtInArray;

public:
	fingerprintArray() {
		uint32_t bitNum = fingerprintNum * fingerprintBitSize;
		cellSize = sizeof(T);
		cellNum = bitNum / cellSize + (bitNum % (cellSize * 8) != 0);
		arraySize = cellSize * cellNum;
		builtInArray = new T[cellNum]();
	}

	uint64_t *getPosition(uint32_t &startPosition, uint32_t &startBit) {
		startPosition = index * fingerprintBitSize / cellSize;
		assert(startPosition < cellNum);
		startBit = index * fingerprintBitSize % cellSize;
		return (uint64_t *)&builtInArray[startPosition];
	}

	uint64_t get(uint32_t index) {
		uint32_t startPosition;
		uint32_t startBit;
		uint64_t mask = ((1 << (64 - startBit)) - 1);
		uint64_t *startPointer = getPosition(startPosition, startBit, mask);

		return (*startPointer & mask) >> (64 - startBit - fingerprintBitSize);
	}

	uint64_t *clear(uint32_t index) {
		uint32_t startPosition;
		uint32_t startBit;
		uint64_t *startPointer = getPosition(startPosition, startBit);
		uint64_t mask = ~((1 << (64 - startBit)) - 1) - ((1 << (64 - startBit - fingerprintBitSize)) - 1);

		(*startPointer) &= mask;

		return startPointer;
	}

	void assign(uint32_t index, uint64_t fingerprint) {
		uint64_t *startPointer = clear(index);
		(*startPointer) ^= (fingerprint << (64 - startBit - fingerprintBitSize));
	}
}

template<typenanme T, uint32_t fingerprintNum, uint32_t fingerprintBitSize>
class cuckooFilter {
	fingerprintArray<T, fingerprintNum, fingerprintBitSize> fingerprints[BUCKET_NUM];
	int replacementCounter = 0;
	uint64_t h, s, n, hf, sf, nf;

	uint32_t calcPosition(Key_t key) {

	}

	uint32_t calcFingerprint(Key_t key) {

	}

	uint32_t calcFingerprintHash(uint64_t fingerprint) {

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
	}

	void insert(Key_t key) {
		uint32_t position1 = calcPosition(key);
		uint64_t fingerprint = calcFingerprint(key), tmpFingerprint;
		int index = -1;
		int ret;
		while ((ret = hasEmptyBucket(position1, index)) == 0) {
			tmpFingerprint = fingerprints[0].get(position1)
			fingerprints[0].assign(position1, fingerprint);
			position1 = position1 ^ calcFingerprintHash(tmpFingerprint);
			fingerprint = tmpFingerprint;
			replacementCounter++;
			if (replacementCounter <= MAX_TRY) {
				std::cout << "Too many replacements!" << std::endl;
				return;
			}
		}
		if (ret == 1)
			fingerprints[index].assign(position1, fingerprint);
	}

	bool query(Key_t key) {
		uint32_t position1 = calcPosition(key);
		uint64_t fingerprint = calcFingerprint(key);
		uint32_t position2 = position1 ^ calcFingerprintHash(fingerprint);

		if ((ret = hasEmptyBucket(position1, index)) == 2) {
			return true;
		}

		return false;
	}

	double getLoadRate() {

	}
};

#endif