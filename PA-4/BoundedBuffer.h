#ifndef BoundedBuffer_h
#define BoundedBuffer_h

#include <stdio.h>
#include <queue>
#include <string>
#include <pthread.h>
using namespace std;

class BoundedBuffer {
private:
	queue<string> q;
	int cap;
	pthread_mutex_t mtx;
	pthread_cond_t cond;
	pthread_cond_t cond2;
public:
    BoundedBuffer(int);
	~BoundedBuffer();
	int size();
    void push (string);
    string pop();
};

#endif /* BoundedBuffer_ */
