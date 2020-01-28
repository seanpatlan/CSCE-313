#include <cassert>
#include <cstring>
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <errno.h>

#include "RequestChannel.h" // for EXITONERROR()
#include "KernelSemaphore.h"
using namespace std;

KernelSemaphore::KernelSemaphore(short _val, key_t key) {
    if ( (semid = semget(key, 1, IPC_CREAT | IPC_EXCL | 0666)) < 0 )
        semid = semget(key, 1, 0666);
    else {
        struct sembuf sb = {0,_val,0};
        semop(semid,&sb,1);
    }
}

KernelSemaphore::~KernelSemaphore() {
    semctl(semid,0,IPC_RMID);
}

void KernelSemaphore::P() {
    struct sembuf sb = {0,-1,0};
    semop(semid,&sb,1);
}

void KernelSemaphore::V() {
    struct sembuf sb = {0,1,0};
    semop(semid,&sb,1);
}
