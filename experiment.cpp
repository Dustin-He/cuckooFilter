#include "cuckooFilter.h"
#include "BloomFilter.h"

using namespace std;

const char *path = "./data/test-8s.dat";
vector<trace_t> traces;
set<trace_t> positiveSample;
set<trace_t> negativeSample;
cuckooFilter<uint64_t, FINGERPRINTNUM, FINGERPRINTBITSIZE> cf;
BloomFilter<BLOOM_SIZE, BLOOM_HASH_NUM> bf;

int readTraces(const char *path) {
	FILE *inputData = fopen(path, "rb");
	traces.clear();
	char *strData = new char[KEY_T_SIZE];

	printf("Reading in data\n");

	while (fread(strData, KEY_T_SIZE, 1, inputData) == 1) {
		traces.push_back(trace_t(strData));
	}
	fclose(inputData);
	
	int size = traces.size();

	printf("Successfully read in %d packets\n", size);

	return size;
}

int main() {
	//read the traces
	int size = readTraces(path);
	int dataSize = 100000;
	int break_number = dataSize;
	int break_number_negative = dataSize;
	int failCnt = 0;
	int right = 0, wrong = 0, right2 = 0, wrong2 = 0;
	bool lastFlow = false;

	/********************* Cuckoo Filter ***************************/

	//Get the ground truth & insert
	for (int i = 0; i < size; ++i) {
		if (!lastFlow) {
			if (positiveSample.find(traces[i]) == positiveSample.end()) {
				positiveSample.insert(traces[i]);
				int ret = cf.insert((Key_t)traces[i].str);
				bf.setbit((Key_t)traces[i].str);
				if (--break_number == 0)
					lastFlow = true;
				if (ret) {
					failCnt++;
				}
					
			}
		}
		else {
			if (positiveSample.find(traces[i]) == positiveSample.end() && negativeSample.find(traces[i]) == negativeSample.end()) {
				negativeSample.insert(traces[i]);
				if (--break_number_negative == 0)
					break;
			}
				
		}
	}
	cout << "Insert " << positiveSample.size() << " positive samples and " << negativeSample.size() << " negative samples" << endl;
	cout << failCnt << " flows not inserted successfully" << endl;

	//Query
	// for (auto it = positiveSample.begin(); it != positiveSample.end(); it++) {
	// 	bool ans = cf.query((Key_t)it->str);
	// 	bool ans2 = bf.getbit((Key_t)it->str);
	// 	if (ans)
	// 		right++;
	// 	else
	// 		wrong++;
	// 	if (ans2)
	// 		right2++;
	// 	else
	// 		wrong2++;
	// }

	// cout << "----- positive samples -----" << endl;
	// cout << "Right: " << right << endl;
	// cout << "Wrong: " << wrong << endl;

	right = wrong = 0;
	for (auto it = negativeSample.begin(); it != negativeSample.end(); it++) {
		bool ans = cf.query((Key_t)it->str);
		bool ans2 = bf.getbit((Key_t)it->str);
		if (!ans)
			right++;
		else
			wrong++;
		if (!ans2)
			right2++;
		else
			wrong2++;
	}

	size_t cfSize = cf.getMemoryUsage();
	size_t bfSize = bf.getMemoryUsage();

	cout << "----- negative samples -----" << endl;
	cout << "Right for CF: " << (double)right / dataSize << endl;
	cout << "Wrong for CF: " << (double)wrong / dataSize << endl;
	cout << "Right for BF: " << (double)right2 / dataSize << endl;
	cout << "Wrong for BF: " << (double)wrong2 / dataSize << endl;
	cout << "Cuckoo Filter Size:" << cfSize / (double)1024 << "KB" << endl;
	cout << "Bloom Filter Size:" << bfSize / (double)1024 << "KB" << endl;
	cout << "Cuckoo Filter Load Rate:" << cf.getLoadRate() << endl;

	return 0;
}




