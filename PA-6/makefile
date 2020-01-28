# makefile

all: dataserver client

clean:
	rm -rf *.o fifo* msq_* shm_* dataserver client

################################################################################

RequestChannel.o: RequestChannel.h RequestChannel.cpp
	g++ -g -w -Wall -O1 -std=c++11 -c RequestChannel.cpp

FIFORequestChannel.o: FIFORequestChannel.h FIFORequestChannel.cpp
	g++ -g -w -Wall -O1 -std=c++11 -c FIFORequestChannel.cpp

MQRequestChannel.o: MQRequestChannel.h MQRequestChannel.cpp
	g++ -g -w -Wall -O1 -std=c++11 -c MQRequestChannel.cpp

SHMRequestChannel.o: SHMRequestChannel.h SHMRequestChannel.cpp
	g++ -g -w -Wall -O1 -std=c++11 -c SHMRequestChannel.cpp

################################################################################

BoundedBuffer.o: BoundedBuffer.h BoundedBuffer.cpp
	g++ -g -w -Wall -O1 -std=c++11 -c BoundedBuffer.cpp

Histogram.o: Histogram.h Histogram.cpp
	g++ -g -w -Wall -O1 -std=c++11 -c Histogram.cpp

KernelSemaphore.o: KernelSemaphore.h KernelSemaphore.cpp
	g++ -g -w -Wall -O1 -std=c++11 -c KernelSemaphore.cpp

SHMBoundedBuffer.o: SHMBoundedBuffer.h SHMBoundedBuffer.cpp
	g++ -g -w -Wall -O1 -std=c++11 -c SHMBoundedBuffer.cpp

################################################################################

dataserver: dataserver.cpp RequestChannel.o FIFORequestChannel.o MQRequestChannel.o SHMRequestChannel.o SHMBoundedBuffer.o KernelSemaphore.o
	g++ -g -w -Wall -O1 -std=c++11 -o dataserver dataserver.cpp RequestChannel.o FIFORequestChannel.o MQRequestChannel.o SHMRequestChannel.o SHMBoundedBuffer.o KernelSemaphore.o -lpthread

client: client.cpp RequestChannel.o FIFORequestChannel.o MQRequestChannel.o SHMRequestChannel.o SHMBoundedBuffer.o BoundedBuffer.o Histogram.o
	g++ -g -w -Wall -O1 -std=c++11 -o client client.cpp RequestChannel.o FIFORequestChannel.o MQRequestChannel.o SHMRequestChannel.o SHMBoundedBuffer.o KernelSemaphore.o BoundedBuffer.o Histogram.o -lpthread
