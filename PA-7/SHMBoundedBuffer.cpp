#include <cassert>
#include <cstring>
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <errno.h>

#include "RequestChannel.h" // for EXITONERROR()
#include "SHMBoundedBuffer.h"

using namespace std;

const int SHM_SIZE = 1024;

SHMBoundedBuffer::SHMBoundedBuffer(string file_name) {
	key_t shm_key, full_key, empty_key;
	if ( (shm_key=ftok(file_name.c_str(),0)) < 0 )
		EXITONERROR("shm_ftok");
	if ( (full_key=ftok(file_name.c_str(),1) ) < 0 )
		EXITONERROR("full_ftok");
	if ( (empty_key=ftok(file_name.c_str(),2)) < 0 )
		EXITONERROR("empty_ftok");

	if( (shmid = shmget(shm_key,SHM_SIZE, 0644 | IPC_CREAT)) < 0 )
		EXITONERROR("shmget");

	buf = (char*) shmat(shmid, (void*) 0, 0);

	full = new KernelSemaphore(0,full_key);
	empty = new KernelSemaphore(1,empty_key);
}

SHMBoundedBuffer::~SHMBoundedBuffer() {
	shmdt(buf);
	shmctl(shmid,IPC_RMID,NULL);

	delete full;
	delete empty;
}

const int MAX_MESSAGE = 1024;

void SHMBoundedBuffer::push(string msg) {
	empty->P();
	strcpy(buf,msg.c_str());
	full->V();
}

string SHMBoundedBuffer::pop() {
	char msg[MAX_MESSAGE];

	full->P();
	strcpy(msg,buf);
	empty->V();

	string str = msg;
	return str;
}
