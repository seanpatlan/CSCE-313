
#ifndef _MQReqChannel_H_
#define _MQReqChannel_H_

#include <iostream>
#include <fstream>
#include <exception>
#include <string>
#include <sys/msg.h>

#include "RequestChannel.h"

class MQRequestChannel: public RequestChannel{

private:
	int msqid;
	string file_name;

public:

	/* -- CONSTRUCTOR/DESTRUCTOR */

	MQRequestChannel(const string _name, const Side _side);
	/* Creates a "local copy" of the channel specified by the given name.
	 If the channel does not exist, the associated IPC mechanisms are
	 created. If the channel exists already, this object is associated with the channel.
	 The channel has two ends, which are conveniently called "SERVER_SIDE" and "CLIENT_SIDE".
	 If two processes connect through a channel, one has to connect on the server side
	 and the other on the client side. Otherwise the results are unpredictable.

	 NOTE: If the creation of the request channel fails (typically happens when too many
	 request channels are being created) and error message is displayed, and the program
	 unceremoniously exits.

	 NOTE: It is easy to open too many request channels in parallel. In most systems,
	 limits on the number of open files per process limit the number of established
	 request channels to 125.
	*/

	~MQRequestChannel();
	/* Destructor of the local copy of the bus. By default, the Server Side deletes any IPC
	 mechanisms associated with the channel. */

	string cread();
	/* Blocking read of data from the channel. Returns a string of characters
	 read from the channel. Returns NULL if read failed. */

	void cwrite(string _msg);
	/* Write the data to the channel. The function returns the number of characters written
	 to the channel. */

	string name();
	/* Returns the name of the request channel. */



};


#endif
