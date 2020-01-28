#include <cassert>
#include <cstring>
#include <sstream>
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>

#include <pthread.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>

#include "RequestChannel.h"
#include "FIFORequestChannel.h"
#include "MQRequestChannel.h"
#include "SHMRequestChannel.h"
#include "NetworkRequestChannel.h"
#include <pthread.h>
using namespace std;

/*** THIS DATASERVER PROGRAM WILL ONLY WORK WITH NETWORK REQUEST CHANNELS ***/


int nchannels = 0;
pthread_mutex_t newchannel_lock;
void* handle_process_loop (void* _channel);

void process_newchannel(RequestChannel* _channel) {
	nchannels ++;
	string new_channel_name = "data" + to_string(nchannels) + "_";
	_channel->cwrite(new_channel_name);

	RequestChannel* data_channel = (RequestChannel*) new NetworkRequestChannel(new_channel_name,RequestChannel::SERVER_SIDE,_channel->socket_fd());
	pthread_t thread_id;
	if (pthread_create(& thread_id, NULL, handle_process_loop, data_channel) < 0 ) {
		EXITONERROR ("");
	}

}

void process_request(RequestChannel* _channel, string _request) {

	if (_request.compare(0, 5, "hello") == 0) {
		_channel->cwrite("hello to you too");
	}
	else if (_request.compare(0, 4, "data") == 0) {
		usleep(1000 + (rand() % 5000));
		_channel->cwrite(to_string(rand() % 100));
	}
	else if (_request.compare(0, 10, "newchannel") == 0) {
		process_newchannel(_channel);
	}
	else {
		_channel->cwrite("unknown request");
	}
}

void* handle_process_loop (void* _channel) {
	RequestChannel* channel = (RequestChannel*) _channel;
	for(;;) {
		string request = channel->cread();
		if (request.compare("quit") == 0) {
			break;                  // break out of the loop;
		}
		process_request(channel, request);
	}
}


/*--------------------------------------------------------------------------*/
/* MAIN FUNCTION */
/*--------------------------------------------------------------------------*/


int main(int argc, char * argv[]) {
	string port = "22";
    int opt = 0;
    while ((opt = getopt(argc, argv, "p:")) != -1) {
        switch (opt) {
			case 'p':
				port = optarg;
				break;
		}
    }

	RequestChannel* control_channel;

	newchannel_lock = PTHREAD_MUTEX_INITIALIZER;

	control_channel = (RequestChannel*) new NetworkRequestChannel("control",RequestChannel::SERVER_SIDE,"",port);

	string slave_channel_name;
	while (true) {
		int slave_fd = accept(control_channel->socket_fd(),NULL,NULL);
		slave_channel_name = "slave_" + to_string(slave_fd);
		RequestChannel* slave_channel = (RequestChannel*) new NetworkRequestChannel(slave_channel_name,RequestChannel::SERVER_SIDE,slave_fd);

		pthread_t slave_thread;
		if ( pthread_create(&slave_thread,NULL,handle_process_loop,slave_channel) < 0 )
			EXITONERROR("");
	}
}
