/*
    File: dataserver.C

    Author: R. Bettati
            Department of Computer Science
            Texas A&M University
    Date  : 2012/07/16

    Dataserver main program for MP3 in CSCE 313
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
#include <sstream>
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>

#include <pthread.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>

#include "reqchannel.h"

/*--------------------------------------------------------------------------*/
/* DATA STRUCTURES */
/*--------------------------------------------------------------------------*/

    /* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* CONSTANTS */
/*--------------------------------------------------------------------------*/

    /* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* VARIABLES */
/*--------------------------------------------------------------------------*/

static int nthreads = 0;

/*--------------------------------------------------------------------------*/
/* FORWARDS */
/*--------------------------------------------------------------------------*/

void handle_process_loop(RequestChannel & _channel);

/*--------------------------------------------------------------------------*/
/* LOCAL FUNCTIONS -- SUPPORT FUNCTIONS */
/*--------------------------------------------------------------------------*/

	/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* LOCAL FUNCTIONS -- THREAD FUNCTIONS */
/*--------------------------------------------------------------------------*/

void * handle_data_requests(void * args) {

	RequestChannel * data_channel =  (RequestChannel*)args;

	// -- Handle client requests on this channel.

	handle_process_loop(*data_channel);

	// -- Client has quit. We remove channel.

	delete data_channel;

	return nullptr;
}

/*--------------------------------------------------------------------------*/
/* LOCAL FUNCTIONS -- INDIVIDUAL REQUESTS */
/*--------------------------------------------------------------------------*/

void process_hello(RequestChannel & _channel, const std::string & _request) {
	_channel.cwrite("hello to you too");
}

void process_data(RequestChannel & _channel, const std::string &  _request) {
	usleep(1000 + (rand() % 5000));
	//_channel.cwrite("here comes data about " + _request.substr(4) + ": " + std::to_string(random() % 100));
	_channel.cwrite(std::to_string(rand() % 100));
}

void process_newthread(RequestChannel & _channel, const std::string & _request) {
  int error;
  nthreads ++;

  // -- Name new data channel

  std::string new_channel_name = "data" + std::to_string(nthreads) + "_";
  //  std::cout << "new channel name = " << new_channel_name << endl;

  // -- Pass new channel name back to client

  _channel.cwrite(new_channel_name);

  // -- Construct new data channel (pointer to be passed to thread function)
	try {
		RequestChannel * data_channel = new RequestChannel(new_channel_name, RequestChannel::SERVER_SIDE);

		// -- Create new thread to handle request channel

		pthread_t thread_id;
		//  std::cout << "starting new thread " << nthreads << endl;

		/*
		 * Dataserver threads need to be detached (preferably,
		 * created in a detached state) since there's no
		 * real way to pthread_join them.
		 */

		if ((errno = pthread_create(& thread_id, NULL, handle_data_requests, data_channel)) != 0) {
			perror(std::string("DATASERVER: " + _channel.name() + ": pthread_create failure").c_str());
			delete data_channel;
		}
	}
	catch (sync_lib_exception sle) {
		perror(std::string(sle.what()).c_str());
	}

}

/*--------------------------------------------------------------------------*/
/* LOCAL FUNCTIONS -- THE PROCESS REQUEST LOOP */
/*--------------------------------------------------------------------------*/

void process_request(RequestChannel & _channel, const std::string & _request) {

	if (_request.compare(0, 5, "hello") == 0) {
		process_hello(_channel, _request);
	}
	else if (_request.compare(0, 4, "data") == 0) {
		process_data(_channel, _request);
	}
	else if (_request.compare(0, 9, "newthread") == 0) {
		process_newthread(_channel, _request);
	}
	else {
		_channel.cwrite("unknown request");
	}
}

void handle_process_loop(RequestChannel & _channel) {

	for(;;) {
		//std::cout << "Reading next request from channel (" << _channel.name() << ") ..." << std::flush;
		std::cout << std::flush;
		std::string request = _channel.cread();
		//std::cout << " done (" << _channel.name() << ")." << endl;
		//std::cout << "New request is " << request << endl;

		if (request.compare("quit") == 0) {
			_channel.cwrite("bye");
			usleep(10000);          // give the other end a bit of time.
			break;                  // break out of the loop;
		}

		process_request(_channel, request);
	}
}

void * invoke_handler(void * arg) {
	RequestChannel _channel = *((RequestChannel*)arg);
	handle_process_loop(_channel);
}

/*--------------------------------------------------------------------------*/
/* MAIN FUNCTION */
/*--------------------------------------------------------------------------*/


int main(int argc, char * argv[]) {
	//  std::cout << "Establishing control channel... " << std::flush;
	RequestChannel control_channel("control", RequestChannel::SERVER_SIDE);
	//  std::cout << "done.\n" << std::flush;
	pthread_t dataserver_tid;
	pthread_create(&dataserver_tid, NULL, invoke_handler, (void*) &control_channel);
	pthread_join(dataserver_tid, NULL);
}

