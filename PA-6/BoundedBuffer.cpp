#include "BoundedBuffer.h"
#include <string>
#include <queue>
#include <iostream>
using namespace std;

BoundedBuffer::BoundedBuffer(int _cap) : cap(_cap) {
	pthread_cond_init(&cond,NULL);
	pthread_cond_init(&cond2,NULL);
	pthread_mutex_init(&mtx,NULL);
}

BoundedBuffer::~BoundedBuffer() {
	pthread_cond_destroy(&cond);
	pthread_cond_destroy(&cond2);
	pthread_mutex_destroy(&mtx);
}

int BoundedBuffer::size() {
	pthread_mutex_lock(&mtx);
	int sz = q.size();
	pthread_mutex_unlock(&mtx);

	return sz;
}

void BoundedBuffer::push(string str) {
	pthread_mutex_lock(&mtx);
	while (q.size() >= cap)
		pthread_cond_wait(&cond,&mtx);

	q.push (str);

	pthread_cond_signal(&cond2);
	pthread_mutex_unlock(&mtx);
}

string BoundedBuffer::pop() {
	pthread_mutex_lock(&mtx);
	while (q.size() <= 0)
		pthread_cond_wait(&cond2,&mtx);

	string s = q.front();
	q.pop();

	pthread_cond_signal(&cond);
	pthread_mutex_unlock(&mtx);

	return s;
}
