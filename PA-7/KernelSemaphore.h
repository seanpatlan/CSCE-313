#ifndef _KernelSemaphoreH_
#define _KernelSemaphoreH_

#include <iostream>
#include <sys/sem.h>
using  namespace std;

class KernelSemaphore {
private:
    int semid;

public:
    KernelSemaphore(short _val, key_t key);
    ~KernelSemaphore();

    void P();
    void V();
};

#endif
