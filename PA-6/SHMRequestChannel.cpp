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
#include <sys/shm.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <errno.h>

#include "RequestChannel.h"
#include "SHMRequestChannel.h"

/*--------------------------------------------------------------------------*/
/* CONSTRUCTOR/DESTRUCTOR FOR CLASS   R e q u e s t C h a n n e l  */
/*--------------------------------------------------------------------------*/

SHMRequestChannel::SHMRequestChannel(const std::string _name, const RequestChannel::Side _side) : RequestChannel(_name,_side)
{
	client_file_name = "shm_" + my_name + "_client";
	server_file_name = "shm_" + my_name + "_server";

	creat(client_file_name.c_str(),NULL);
	creat(server_file_name.c_str(),NULL);

	client_buffer = new SHMBoundedBuffer(client_file_name);
	server_buffer = new SHMBoundedBuffer(server_file_name);
}

SHMRequestChannel::~SHMRequestChannel() {
	delete client_buffer;
	delete server_buffer;

	remove(client_file_name.c_str());
	remove(server_file_name.c_str());
}

string SHMRequestChannel::cread() {
	string str;
	if (my_side == CLIENT_SIDE) str = server_buffer->pop();
	else str = client_buffer->pop();

	return str;
}

void SHMRequestChannel::cwrite(string msg) {
	if (my_side == CLIENT_SIDE) client_buffer->push(msg);
	else server_buffer->push(msg);
}

std::string SHMRequestChannel::name() {
	return my_name;
}
