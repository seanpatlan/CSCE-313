#include <cassert>
#include <cstring>
#include <sstream>
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>

#include <pthread.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>

#include "RequestChannel.h"
#include "FIFORequestChannel.h"
#include "MQRequestChannel.h"
#include "SHMRequestChannel.h"
#include <pthread.h>
using namespace std;


int nchannels = 0;
pthread_mutex_t newchannel_lock;
void* handle_process_loop (void* args);

struct loop_args {
	RequestChannel* channel;
	string ipc;

	loop_args(RequestChannel* _c, string _i) : channel(_c), ipc(_i) { }
};

void process_newchannel(RequestChannel* _channel, string ipc) {
	nchannels ++;
	string new_channel_name = "data" + to_string(nchannels) + "_";
	_channel->cwrite(new_channel_name);

	RequestChannel * data_channel;
	if (ipc == "f") data_channel = (RequestChannel*) new FIFORequestChannel(new_channel_name, RequestChannel::SERVER_SIDE);
	if (ipc == "q") data_channel = (RequestChannel*) new MQRequestChannel(new_channel_name, RequestChannel::SERVER_SIDE);
	if (ipc == "s") data_channel = (RequestChannel*) new SHMRequestChannel(new_channel_name, RequestChannel::SERVER_SIDE);

	loop_args* args = new loop_args(data_channel,ipc);
	pthread_t thread_id;
	if (pthread_create(& thread_id, NULL, handle_process_loop, args) < 0 ) {
		EXITONERROR ("");
	}

}

void process_request(RequestChannel* _channel, string _request, string ipc) {

	if (_request.compare(0, 5, "hello") == 0) {
		_channel->cwrite("hello to you too");
	}
	else if (_request.compare(0, 4, "data") == 0) {
		usleep(1000 + (rand() % 5000));
		_channel->cwrite(to_string(rand() % 100));
	}
	else if (_request.compare(0, 10, "newchannel") == 0) {
		process_newchannel(_channel, ipc);
	}
	else {
		_channel->cwrite("unknown request");
	}
}

void* handle_process_loop (void* args) {
	loop_args* p = (loop_args*) args;
	RequestChannel* channel = (RequestChannel*) p->channel;
	for(;;) {
		string request = channel->cread();
		if (request.compare("quit") == 0) {
			break;                  // break out of the loop;
		}
		process_request(channel, request, p->ipc);
	}
}


/*--------------------------------------------------------------------------*/
/* MAIN FUNCTION */
/*--------------------------------------------------------------------------*/


int main(int argc, char * argv[]) {
	string ipc = "f";
    int opt = 0;
    while ((opt = getopt(argc, argv, "i:")) != -1) {
        switch (opt) {
            case 'i':
            	ipc = optarg;
            	break;
		}
    }

	RequestChannel* control_channel;

	newchannel_lock = PTHREAD_MUTEX_INITIALIZER;

	if (ipc == "f") control_channel = (RequestChannel*) new FIFORequestChannel("control", RequestChannel::SERVER_SIDE);
	if (ipc == "q") control_channel = (RequestChannel*) new MQRequestChannel("control", RequestChannel::SERVER_SIDE);
	if (ipc == "s") control_channel = (RequestChannel*) new SHMRequestChannel("control", RequestChannel::SERVER_SIDE);

	loop_args* args = new loop_args(control_channel,ipc);
	handle_process_loop (&control_channel);
}
