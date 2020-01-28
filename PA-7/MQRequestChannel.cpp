/*
    File: requestchannel.C

    Author: R. Bettati
            Department of Computer Science
            Texas A&M University
    Date  : 2012/07/11

*/

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

    /* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include <cassert>
#include <cstring>
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <errno.h>

#include "MQRequestChannel.h"

/*--------------------------------------------------------------------------*/
/* CONSTRUCTOR/DESTRUCTOR FOR CLASS   R e q u e s t C h a n n e l  */
/*--------------------------------------------------------------------------*/

MQRequestChannel::MQRequestChannel(const string _name, const Side _side) : RequestChannel(_name,_side) {
	// creates a file for the message queue
	file_name = "msq_" + my_name;
	creat(file_name.c_str(),NULL);

	// creates a key and ID for the message queue
	key_t key;
	if ( (key = ftok(file_name.c_str(),100)) < 0 )
		EXITONERROR("ftok");

	if ( (msqid = msgget(key, 0644 | IPC_CREAT)) < 0 )
		EXITONERROR("msgget");
}

MQRequestChannel::~MQRequestChannel() {
	// deletes the message queue and removes the previously created file
    msgctl(msqid,IPC_RMID,NULL);
    remove(file_name.c_str());
}

const long CLIENT_TYPE = 1;
const long SERVER_TYPE = 2;

struct msg_buf {
	    long msg_type;
	    char msg[200];
};

string MQRequestChannel::cread() {
	// decides which type the program will read based on the side of the channel
	long read_type;
	if (my_side == CLIENT_SIDE) read_type = SERVER_TYPE;
	else read_type = CLIENT_TYPE;

	// places the message into a buffer
	struct msg_buf buf;
	if ( msgrcv(msqid,&buf,sizeof(buf.msg),read_type,0) < 0 )
		EXITONERROR("cread");

	// returns the message as a string
	string str = buf.msg;
	return str;
}

void MQRequestChannel::cwrite(string _msg) {
	// places the message into a buffer
	struct msg_buf buf;
	strcpy(buf.msg,_msg.c_str());

	// establishes the type of message
	if (my_side == CLIENT_SIDE) buf.msg_type = CLIENT_TYPE;
	else buf.msg_type = SERVER_TYPE;

	// sends the message to the queue
	if ( msgsnd(msqid,&buf,strlen(buf.msg)+1,0) < 0 )
		EXITONERROR("cwrite");
}

std::string MQRequestChannel::name() {
	return my_name;
}
