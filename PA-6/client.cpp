/*
    Based on original assignment by: Dr. R. Bettati, PhD
    Department of Computer Science
    Texas A&M University
    Date  : 2013/01/31
 */


#include <iostream>
#include <fstream>
#include <cstring>
#include <string>
#include <sstream>
#include <iomanip>

#include <sys/time.h>
#include <cassert>
#include <assert.h>

#include <cmath>
#include <numeric>
#include <algorithm>

#include <list>
#include <vector>
#include <unordered_map>

#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>
#include <pthread.h>
#include <chrono>

#include "RequestChannel.h"
#include "FIFORequestChannel.h"
#include "MQRequestChannel.h"
#include "SHMRequestChannel.h"
#include "BoundedBuffer.h"
#include "Histogram.h"

#include <sstream>

using namespace std;

/*********************** REQUEST THREADS ***********************/

struct request_thread_args {
	string patient_name;
	BoundedBuffer* request_buffer;
	int n;
	request_thread_args(string pn, BoundedBuffer* rb, int _n) : patient_name(pn), request_buffer(rb), n(_n) {}
};

void* request_thread_function(void* arg) {
	request_thread_args* p = (request_thread_args*) arg;

	for (int i = 0; i < p->n; i++) {
		p->request_buffer->push(p->patient_name);
	}

	delete arg;
	return NULL;
}

/*********************** WORKER THREADS ***********************/

struct worker_thread_args {
	BoundedBuffer* request_buffer;
	RequestChannel* workerChannel;
	unordered_map<string,BoundedBuffer*>* stat_buffer;

	worker_thread_args(BoundedBuffer* rb, RequestChannel* wc, unordered_map<string,BoundedBuffer*>* sb)
		: request_buffer(rb), workerChannel(wc), stat_buffer(sb) { }
};

void* worker_thread_function(void* arg) {
	worker_thread_args* p = (worker_thread_args*) arg;
 	while (true) {
 		string request = p->request_buffer->pop();
 		p->workerChannel->cwrite(request);

 		if(request == "quit") {
 			delete p->workerChannel;
 			break;
 		}else{
 			string response = p->workerChannel->cread();
 			p->stat_buffer->at(request)->push(response);
 		}
 	}

	delete arg;
	return NULL;
}

/************************ STAT THREADS ************************/

struct stat_thread_args {
	unordered_map<string,BoundedBuffer*>* stat_buffer;
	Histogram* hist;
	string request;
	int n;

	stat_thread_args(unordered_map<string,BoundedBuffer*>* sb, Histogram* h, string r, int _n)
		: stat_buffer(sb), hist(h), request(r), n(_n) { }
};

void* stat_thread_function(void* arg) {
	stat_thread_args* p = (stat_thread_args*) arg;

    for (int i = 0; i < p->n; i++) {
		string response = p->stat_buffer->at(p->request)->pop();
		p->hist->update(p->request,response);
    }

	delete p->stat_buffer->at(p->request);
	delete arg;
	return NULL;
}

/************************ SIGNAL HANDLER ************************/

struct handler_args {
	Histogram* hist;
	bool* stop;
	handler_args(Histogram* h, bool* s) : hist(h), stop(s) { }
};

void* handler(void* args) {
	handler_args* p = (handler_args*) args;
	while (*(p->stop)) {
		// clears the console and prints the histogram
		system("clear");
		p->hist->print();

		// wait until 2s pass or the program ends
		auto start = chrono::high_resolution_clock::now();
		auto finish = chrono::high_resolution_clock::now();
		while (*(p->stop) && (chrono::duration_cast<chrono::nanoseconds>(finish-start).count()/1e9 < 2))
			finish = chrono::high_resolution_clock::now();
	}
	system("clear");

	return NULL;
}


/*--------------------------------------------------------------------------*/
/* MAIN FUNCTION */
/*--------------------------------------------------------------------------*/

int main(int argc, char * argv[]) {
    int n = 100; //default number of requests per "patient"
    int w = 1; //default number of worker threads
    int b = 3 * n; // default capacity of the request buffer, you should change this default
    string ipc = "f";
    int opt = 0;
    while ((opt = getopt(argc, argv, "n:w:b:i:")) != -1) {
        switch (opt) {
            case 'n':
                n = atoi(optarg);
                break;
            case 'w':
                w = atoi(optarg); //This won't do a whole lot until you fill in the worker thread function
                break;
            case 'b':
                b = atoi (optarg);
                break;
            case 'i':
            	ipc = optarg;
            	break;
		}
    }

    int pid = fork();
	if (pid == 0){
		execl("dataserver","dataserver","-i",ipc.c_str(), (char*) NULL);
	}
	else {
		auto start = chrono::high_resolution_clock::now();

        cout << "n == " << n << endl;
        cout << "w == " << w << endl;
        cout << "b == " << b << endl;
        cout << "i == " << ipc << endl;

        RequestChannel *chan;
        if (ipc == "f") chan = (RequestChannel*) new FIFORequestChannel("control", RequestChannel::CLIENT_SIDE);
        if (ipc == "q") chan = (RequestChannel*) new MQRequestChannel("control", RequestChannel::CLIENT_SIDE);
        if (ipc == "s") chan = (RequestChannel*) new SHMRequestChannel("control", RequestChannel::CLIENT_SIDE);

        BoundedBuffer request_buffer(b);
		Histogram hist;
		bool stop = true;

		// creates a map of stat buffers for each request
		unordered_map<string,BoundedBuffer*> stat_buffer;
		stat_buffer["data John Smith"] = new BoundedBuffer(b/3);
		stat_buffer["data Jane Smith"] = new BoundedBuffer(b/3);
		stat_buffer["data Joe Smith"] = new BoundedBuffer(b/3);

		// creates request argument structs for the three names
		request_thread_args* john_req_args = new request_thread_args("data John Smith",&request_buffer,n);
		request_thread_args* jane_req_args = new request_thread_args("data Jane Smith",&request_buffer,n);
		request_thread_args* joe_req_args = new request_thread_args("data Joe Smith",&request_buffer,n);

		// creates request thread objects for the three names
		pthread_t john_req_pt;
		pthread_t jane_req_pt;
		pthread_t joe_req_pt;

		// creates stat thread objects for the three names
		pthread_t john_stat_pt;
		pthread_t jane_stat_pt;
		pthread_t joe_stat_pt;

		// creates stat argument structs for the three names
		stat_thread_args* john_stat_args = new stat_thread_args(&stat_buffer,&hist,"data John Smith",n);
		stat_thread_args* jane_stat_args = new stat_thread_args(&stat_buffer,&hist,"data Jane Smith",n);
		stat_thread_args* joe_stat_args = new stat_thread_args(&stat_buffer,&hist,"data Joe Smith",n);

		// creates stat threads for the three names
		pthread_create(&john_stat_pt,NULL,stat_thread_function,john_stat_args);
		pthread_create(&jane_stat_pt,NULL,stat_thread_function,jane_stat_args);
		pthread_create(&joe_stat_pt,NULL,stat_thread_function,joe_stat_args);

		// creates threads for the three names
		pthread_create(&john_req_pt,NULL,request_thread_function,john_req_args);
		pthread_create(&jane_req_pt,NULL,request_thread_function,jane_req_args);
		pthread_create(&joe_req_pt,NULL,request_thread_function,joe_req_args);

		pthread_t worker_threads[w];
		worker_thread_args* args[w];
		string s;
		for (int i = 0; i < w; i++) {
			chan->cwrite("newchannel");
			s = chan->cread ();

			RequestChannel* workerChannel;
	        if (ipc == "f") workerChannel = (RequestChannel*) new FIFORequestChannel(s, RequestChannel::CLIENT_SIDE);
	        if (ipc == "q") workerChannel = (RequestChannel*) new MQRequestChannel(s, RequestChannel::CLIENT_SIDE);
	        if (ipc == "s") workerChannel = (RequestChannel*) new SHMRequestChannel(s, RequestChannel::CLIENT_SIDE);

			args[i] = new worker_thread_args(&request_buffer,workerChannel,&stat_buffer);
			worker_threads[i] = pthread_t();
			pthread_create(&worker_threads[i],NULL,worker_thread_function,args[i]);
		}

		// creates a thread for the signal handler
		pthread_t sig_pt;
		handler_args* sig_args = new handler_args(&hist,&stop);
		pthread_create(&sig_pt,NULL,handler,sig_args);

		// join request threads
		pthread_join(john_req_pt,NULL);
		pthread_join(jane_req_pt,NULL);
		pthread_join(joe_req_pt,NULL);

		cout << "Done populating request buffer" << endl;

        cout << "Pushing quit requests... ";
        for(int i = 0; i < w; ++i) {
            request_buffer.push("quit");
        }
        cout << "done." << endl;

		for (int i = 0; i < w; i++)
			pthread_join(worker_threads[i],NULL);

		pthread_join(john_stat_pt,NULL);
		pthread_join(jane_stat_pt,NULL);
		pthread_join(joe_stat_pt,NULL);

		chan->cwrite ("quit");
		delete chan;
		cout << "All Done!!!" << endl;

		stop = false;
		pthread_join(sig_pt,NULL);

		hist.print ();

		auto finish = chrono::high_resolution_clock::now();
		double t = chrono::duration_cast<chrono::nanoseconds>(finish-start).count()/1e9;

		cout << "\n\n" << t << " s" << endl;
    }
}
