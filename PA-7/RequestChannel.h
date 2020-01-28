
#ifndef _ReqChannel_H_
#define _ReqChannel_H_

#include <iostream>
#include <fstream>
#include <exception>
#include <string>
using namespace std;

void EXITONERROR (string msg);

class RequestChannel {
public:
	typedef enum {SERVER_SIDE, CLIENT_SIDE} Side;
	typedef enum {READ_MODE, WRITE_MODE} Mode;

protected:
	string my_name;
	string side_name;
	Side my_side;

public:
	/* -- CONSTRUCTOR/DESTRUCTOR */
	RequestChannel(const string _name, const Side _side);
	virtual ~RequestChannel();

	virtual string cread() = 0;
	/* Blocking read of data from the channel. Returns a string of characters
	 read from the channel. Returns NULL if read failed. */

	virtual void cwrite(string _msg) = 0;
	/* Write the data to the channel. The function returns the number of characters written
	 to the channel. */

	 virtual int socket_fd() { return 0; }
};

#endif
