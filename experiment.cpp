#include "cuckooFilter.h"

using namespace std;

const char *path = "./data/test-8s.dat";
vector<trace_t> traces;
set<trace_t> positiveSample;
set<trace_t> negativeSample;
cuckooFilter<uint64_t, FINGERPRINTNUM, FINGERPRINTBITSIZE> cf;

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
	int break_number = 100000;
	int break_number_negative = 100000;
	int failCnt = 0;
	int right = 0, wrong = 0;
	bool lastFlow = false;

	/********************* Cuckoo Filter ***************************/

	//Get the ground truth & insert
	for (int i = 0; i < size; ++i) {
		if (!lastFlow) {
			if (positiveSample.find(traces[i]) == positiveSample.end()) {
				positiveSample.insert(traces[i]);
				int ret = cf.insert((Key_t)traces[i].str);
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

	//Query
	for (auto it = positiveSample.begin(); it != positiveSample.end(); it++) {
		bool ans = cf.query((Key_t)it->str);
		if (ans)
			right++;
		else {
			wrong++;
			// for (int i = 0; i < KEY_T_SIZE; ++i)
			// 	cout << (uint32_t)(uint8_t)it->str[i] << " ";
			// cout << endl;
		}
	}

	cout << "----- positive smaples -----" << endl;
	cout << "Right: " << right << endl;
	cout << "Wrong: " << wrong << endl;
	cout << failCnt << " flows not inserted successfully" << endl;

	for (auto it = negativeSample.begin(); it != negativeSample.end(); it++) {
		bool ans = cf.query((Key_t)it->str);
		if (!ans)
			right++;
		else
			wrong++;
	}

	size_t cfSize = cf.getMemoryUsage();

	cout << "----- negative smaples -----" << endl;
	cout << "Right: " << right << endl;
	cout << "Wrong: " << wrong << endl;
	cout << "Cuckoo Filter Size:" << cfSize / (double)1024 << "KB" << endl;

	return 0;
}




