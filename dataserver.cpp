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

#include "netreqchannel.h"

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
// pthread_mutex_t channel_mutex;
/*--------------------------------------------------------------------------*/

static int nthreads = 0;

/*--------------------------------------------------------------------------*/
/* FORWARDS */
/*--------------------------------------------------------------------------*/

void* handle_process_loop(void* _channel);

/*--------------------------------------------------------------------------*/
/* LOCAL FUNCTIONS -- SUPPORT FUNCTIONS */
/*--------------------------------------------------------------------------*/

  /* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* LOCAL FUNCTIONS -- THREAD FUNCTIONS */
/*--------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------*/
/* LOCAL FUNCTIONS -- INDIVIDUAL REQUESTS */
/*--------------------------------------------------------------------------*/

void process_hello(NetworkRequestChannel* _channel, const std::string & _request) {
  _channel->cwrite("hello to you too");
}

void process_data(NetworkRequestChannel* _channel, const std::string &  _request) {
  usleep(1000 + (rand() % 5000));
  int result = rand() % 100;
  // std::cerr << "here comes data about " << _request << ":" << result << ";" << std::endl;
  _channel->cwrite(std::to_string(result));
}

/*--------------------------------------------------------------------------*/
/* LOCAL FUNCTIONS -- THE PROCESS REQUEST LOOP */
/*--------------------------------------------------------------------------*/

void process_request(NetworkRequestChannel* _channel, const std::string & _request) {
  // std::cout << "Get request: " << _request << std::endl;

  if (_request.compare(0, 5, "hello") == 0) {
    process_hello(_channel, _request);
  }
  else if (_request.compare(0, 4, "data") == 0) {
    process_data(_channel, _request);
  }
  // else if (_request.compare(0, 9, "newthread") == 0) {
  //   process_newthread(_channel, _request);
  // }
  else {
    // std::cerr << "Unknown request:" << _request << std::endl;
    _channel->cwrite("unknown request");
  }
}

void* handle_process_loop(void* _channel) {

  NetworkRequestChannel* chan = (NetworkRequestChannel*)_channel;
  for(;;) {
    std::string request = chan->cread();

    if (request.compare("quit") == 0) {
      chan->cwrite("bye");
      usleep(10000);          // give the other end a bit of time.
      break;                  // break out of the loop;
    }

    process_request(chan, request);
  }
}

/*--------------------------------------------------------------------------*/
/* MAIN FUNCTION */
/*--------------------------------------------------------------------------*/


int main(int argc, char * argv[]) {
  int opt = 0;
  int backlog_buffer = 20;
  std::string server_port;
  while ((opt = getopt(argc, argv, "p:b:h")) != -1) {
    switch (opt) {
    case 'p':
      server_port = optarg;
      break;
    case 'b':
      backlog_buffer = atoi(optarg);
      break;
    case 'h':
    default:
      std::cout << "This program can be invoked with the following flags:"
                << std::endl;
      std::cout << "-p [int]: Port number for data server" << std::endl;
      std::cout << "-b [int]: backlog of the server socket" << std::endl;
      exit(0);
    }
  }

   // std::cout << "Establishing control channel... " << std::flush;
  // pthread_mutex_init (&channel_mutex, NULL);
  std::cout << "Starting server at port " << server_port << std::endl;
  std::cout << "backlog buffer = " << backlog_buffer << std::endl;
  NetworkRequestChannel control_channel(server_port, handle_process_loop, backlog_buffer);
  //  std::cout << "done.\n" << std::flush;

  // handle_process_loop(control_channel);
}
