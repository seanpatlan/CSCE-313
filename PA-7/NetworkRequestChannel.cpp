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
#include <sys/socket.h>
#include <netdb.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <errno.h>

#include "RequestChannel.h"
#include "NetworkRequestChannel.h"

/*--------------------------------------------------------------------------*/
/* CONSTRUCTOR/DESTRUCTOR FOR CLASS   R e q u e s t C h a n n e l  */
/*--------------------------------------------------------------------------*/

NetworkRequestChannel::NetworkRequestChannel(const std::string _name, const RequestChannel::Side _side, int new_fd) : RequestChannel(_name,_side)
{
    server_name = "";
    port = "";
    sockfd = new_fd;
}

NetworkRequestChannel::NetworkRequestChannel(const std::string _name, const RequestChannel::Side _side, string _host, string _port) : RequestChannel(_name,_side)
{
    server_name = _host;
    port = _port;

    // SERVER SIDE (taken from the given ServerTCP.cpp)
    if (my_side == RequestChannel::SERVER_SIDE) {
        int new_fd;  // listen on sock_fd, new connection on new_fd
        struct addrinfo hints, *serv;
        struct sockaddr_storage their_addr; // connector's address information
        socklen_t sin_size;
        char s[INET6_ADDRSTRLEN];
        int rv;

        memset(&hints, 0, sizeof hints);
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_flags = AI_PASSIVE; // use my IP

        if ((rv = getaddrinfo(NULL, port.c_str(), &hints, &serv)) != 0) {
            cerr  << "getaddrinfo (server): " << gai_strerror(rv) << endl;
            EXITONERROR("getaddrinfo (server)");
        }
    	if ((sockfd = socket(serv->ai_family, serv->ai_socktype, serv->ai_protocol)) == -1)
            EXITONERROR("server: socket");

        if (bind(sockfd, serv->ai_addr, serv->ai_addrlen) == -1) {
    		close(sockfd);
    		EXITONERROR("server: bind");
    	}
        freeaddrinfo(serv); // all done with this structure

        if (listen(sockfd, 20) == -1)
            EXITONERROR("listen");
    }
    // CLIENT SIDE (taken from the given ClientTCP.cpp)
    else {
        struct addrinfo hints, *res;

    	memset(&hints, 0, sizeof hints);
    	hints.ai_family = AF_UNSPEC;
    	hints.ai_socktype = SOCK_STREAM;
    	int status;

    	if ((status = getaddrinfo(server_name.c_str(), port.c_str(), &hints, &res)) != 0) {
            cerr << "getaddrinfo (client): " << gai_strerror(status) << endl;
            EXITONERROR("getaddrinfo (client)");
        }

    	// make a socket:
    	if ( (sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) < 0 )
    		EXITONERROR("Cannot create scoket");

    	// connect!
    	if (connect(sockfd, res->ai_addr, res->ai_addrlen) < 0)
    		EXITONERROR("Cannot Connect");
    }
}

NetworkRequestChannel::~NetworkRequestChannel() {
    close(sockfd);
}

string NetworkRequestChannel::cread() {
    char msg[1024];
    if (recv(sockfd,msg,sizeof(msg),0) < 0)
        EXITONERROR("cread");

    string str = msg;
    return str;
}

void NetworkRequestChannel::cwrite(string _msg) {
    char msg[1024];
    strcpy(msg,_msg.c_str());

	if (send(sockfd,msg,strlen(msg)+1,0) < 0)
		EXITONERROR("cwrite");
}

std::string NetworkRequestChannel::name() {
	return my_name;
}

int NetworkRequestChannel::socket_fd() {
    return sockfd;
}
