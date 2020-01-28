
#ifndef _SHMBuffer_H_
#define _SHMBuffer_H_

#include "KernelSemaphore.h"

using namespace std;

class SHMBoundedBuffer {
private:
	int shmid;
	char* buf;

	KernelSemaphore* full;
	KernelSemaphore* empty;

public:
	SHMBoundedBuffer(string file_name);
	~SHMBoundedBuffer();

	void push(string msg);
	string pop();
};

#endif
