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
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <errno.h>

#include "reqchannel.h"

/*--------------------------------------------------------------------------*/
/* DATA STRUCTURES */ 
/*--------------------------------------------------------------------------*/

    /* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* CONSTANTS */
/*--------------------------------------------------------------------------*/

const bool VERBOSE = false;

/*--------------------------------------------------------------------------*/
/* FORWARDS */
/*--------------------------------------------------------------------------*/

    /* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* PRIVATE METHODS FOR CLASS   R e q u e s t C h a n n e l  */
/*--------------------------------------------------------------------------*/

std::string RequestChannel::pipe_name(Mode _mode) {
	std::string pname = "fifo_" + my_name;

	if (my_side == CLIENT_SIDE) {
		if (_mode == READ_MODE)
			pname += "1";
		else
			pname += "2";
	}
	else {
	/* SERVER_SIDE */
		if (_mode == READ_MODE)
			pname += "2";
		else
			pname += "1";
	}

	return pname;
}

void RequestChannel::open_write_pipe(const char * _pipe_name) {

	if(write_pipe_opened) close(wfd);

	if(VERBOSE) std::cout << my_name << ":" << side_name << ": mkfifo write pipe" << std::endl;

	if (mkfifo(_pipe_name, 0600) < 0) {
		if (errno != EEXIST) {
			int prev_errno = errno;
			if(read_pipe_opened) close(rfd);
			remove(pipe_name(READ_MODE).c_str());
			remove(pipe_name(WRITE_MODE).c_str());
			errno = prev_errno;
			throw sync_lib_exception(my_name + ":" + side_name + ": error creating pipe for writing");
		}
	}

	if(VERBOSE) std::cout << my_name << ":" << side_name << ": open write pipe" << std::endl;

	wfd = open(_pipe_name, O_WRONLY);
	if (wfd < 0) {
		int prev_errno = errno;
		if(read_pipe_opened) close(rfd);
		remove(pipe_name(READ_MODE).c_str());
		remove(pipe_name(WRITE_MODE).c_str());
		errno = prev_errno;
		throw sync_lib_exception(my_name + ":" + side_name + ": error opening pipe for writing");
	}

	if(VERBOSE) std::cout << my_name << ":" << side_name << ": done opening write pipe" << std::endl;
	write_pipe_opened = true;
}

void RequestChannel::open_read_pipe(const char * _pipe_name) {

	if(read_pipe_opened) close(rfd);
	
	if(VERBOSE) std::cout << my_name << ":" << side_name << ": mkfifo read pipe" << std::endl;

	if (mkfifo(_pipe_name, 0600) < 0) {
		if (errno != EEXIST) {
			int prev_errno = errno;
			if(write_pipe_opened) close(wfd);
			remove(pipe_name(READ_MODE).c_str());
			remove(pipe_name(WRITE_MODE).c_str());
			errno = prev_errno;
			throw sync_lib_exception(my_name + ":" + side_name + ": error creating pipe for reading");
		}
	}

	if(VERBOSE) std::cout << my_name << ":" << side_name << ": open read pipe" << std::endl;

	rfd = open(_pipe_name, O_RDONLY);
	if (rfd < 0) {
		int prev_errno = errno;
		if(write_pipe_opened) close(wfd);
		remove(pipe_name(READ_MODE).c_str());
		remove(pipe_name(WRITE_MODE).c_str());
		errno = prev_errno;
		throw sync_lib_exception(my_name + ":" + side_name + ": error opening pipe for reading");
	}

	if(VERBOSE) std::cout << my_name << ":" << side_name << ": done opening read pipe" << std::endl;
	read_pipe_opened = true;
}

/*--------------------------------------------------------------------------*/
/* CONSTRUCTOR/DESTRUCTOR FOR CLASS   R e q u e s t C h a n n e l  */
/*--------------------------------------------------------------------------*/

RequestChannel::RequestChannel(const std::string _name, const Side _side) :
my_name(_name), my_side(_side), side_name((_side == RequestChannel::SERVER_SIDE) ? "SERVER" : "CLIENT")
{
	//Necessary for proper error handling
	sigset_t sigpipe_set;
	sigemptyset(&sigpipe_set);
	if(sigaddset(&sigpipe_set, SIGPIPE) < 0) {
		throw sync_lib_exception(my_name + ":" + side_name + ": failed on sigaddset(&sigpipe_set, SIGPIPE)");
	}
	
	if((errno = pthread_sigmask(SIG_SETMASK, &sigpipe_set, NULL)) != 0) {
		throw sync_lib_exception(my_name + ":" + side_name + ": failed on pthread_sigmas(SIG_SETMASK, &sigpipe_set, NULL)");
	}
	
	if((errno = pthread_mutexattr_init(&srl_attr)) != 0) {
		throw sync_lib_exception(my_name + ":" + side_name + ": failed on pthread_mutexattr_init");
	}
	if((errno = pthread_mutexattr_setrobust(&srl_attr, PTHREAD_MUTEX_ROBUST)) != 0) {
		throw sync_lib_exception(my_name + ":" + side_name + ": failed on pthread_mutexattr_setrobust");
	}
	if((errno = pthread_mutexattr_setpshared(&srl_attr, PTHREAD_PROCESS_SHARED)) != 0) {
		throw sync_lib_exception(my_name + ":" + side_name + ": failed on pthread_mutexattr_setpshared");
	}
	if((errno = pthread_mutex_init(&send_request_lock, &srl_attr)) != 0) {
		throw sync_lib_exception(my_name + ":" + side_name + ": failed on pthread_mutex_init");
	}
	
	if (_side == SERVER_SIDE) {
		open_write_pipe(pipe_name(WRITE_MODE).c_str());
		open_read_pipe(pipe_name(READ_MODE).c_str());
	}
	else {
		open_read_pipe(pipe_name(READ_MODE).c_str());
		open_write_pipe(pipe_name(WRITE_MODE).c_str());
	}
}

RequestChannel::~RequestChannel() {
	if(VERBOSE) std::cout << my_name << ":" << side_name << ": closing..." << std::endl;
	pthread_mutexattr_destroy(&srl_attr);
	pthread_mutex_destroy(&read_lock);
	pthread_mutex_destroy(&write_lock);
	pthread_mutex_destroy(&send_request_lock);
	close(wfd);
	close(rfd);
	if (my_side == RequestChannel::SERVER_SIDE) {
		//if(VERBOSE) std::cout << "RequestChannel:" << my_name << ":" << side_name << "close IPC mechanisms on server side for channel " << my_name << std::endl;
		/* Destruct the underlying IPC mechanisms. */
		if (remove(pipe_name(READ_MODE).c_str()) != 0 && errno != ENOENT) {
			perror(std::string(my_name + ": Error deleting pipe read pipe").c_str());
		}

		if (remove(pipe_name(WRITE_MODE).c_str()) != 0 && errno != ENOENT) {
			perror(std::string(my_name + ": Error deleting pipe write pipe").c_str());
		}
	}
}

/*--------------------------------------------------------------------------*/
/* READ/WRITE FROM/TO REQUEST CHANNELS  */
/*--------------------------------------------------------------------------*/

const int MAX_MESSAGE = 255;

std::string RequestChannel::send_request(std::string _request) {
	pthread_mutex_lock(&send_request_lock);
	if(cwrite(_request) < 0) {
		pthread_mutex_unlock(&send_request_lock);
		return "ERROR";
	}
	std::string s = cread();
	pthread_mutex_unlock(&send_request_lock);
	return s;
}

std::string RequestChannel::cread() {

	pthread_mutex_lock(&read_lock);
	
	char buf[MAX_MESSAGE];
	memset(buf, '\0', MAX_MESSAGE);

	if(VERBOSE) std::cout << my_name << ":" << side_name << ": reading..." << std::endl;
	
	int read_return_value;
	if ((read_return_value = read(rfd, buf, MAX_MESSAGE)) <= 0) {
		if(read_return_value < 0) {
			perror(std::string(my_name + ":" + side_name + ": error reading from pipe").c_str());
			pthread_mutex_unlock(&read_lock);
			return "ERROR";
		}
		else {
			perror(std::string(my_name + ":" + side_name + ":cread: broken/closed pipe detected, exiting thread...").c_str());
		}
		close(rfd);
		close(wfd);
		remove(pipe_name(READ_MODE).c_str());
		remove(pipe_name(WRITE_MODE).c_str());
		pthread_mutex_unlock(&send_request_lock);
		pthread_mutex_unlock(&read_lock);
		pthread_exit(NULL);
	}
	pthread_mutex_unlock(&read_lock);

	std::string s = buf;

	if(VERBOSE) std::cout << my_name << ":" << side_name << ": reads [" << buf << "]" << std::endl;

	return s;

}

int RequestChannel::cwrite(std::string _msg) {

	if (_msg.length() >= MAX_MESSAGE) {
		if(VERBOSE) std::cerr << my_name << ":" << side_name << "Message too long for Channel!" << std::endl;
		return -1;
	}
	
	pthread_mutex_lock(&write_lock);

	if(VERBOSE) std::cout << my_name << ":" << side_name << ": writing [" << _msg << "]" << std::endl;

	const char * s = _msg.c_str();

	int write_return_value;
	if ((write_return_value = write(wfd, s, strlen(s)+1)) < 0) {
		//EPIPE is part of the normal error-handling process for
		//RequestChannels that are successfully constructed,
		//but that don't have threads enough threads created for them.

		if(errno != EPIPE) {
			perror(std::string(my_name + ":" + side_name + ": error writing to pipe").c_str());
			pthread_mutex_unlock(&write_lock);
			return -1;
		}
		else {
			perror(std::string(my_name + ":" + side_name + ":cwrite: broken/closed pipe detected, exiting worker thread...").c_str());
		}
		close(rfd);
		close(wfd);
		remove(pipe_name(READ_MODE).c_str());
		remove(pipe_name(WRITE_MODE).c_str());
		pthread_mutex_unlock(&send_request_lock);
		pthread_mutex_unlock(&write_lock);
		pthread_exit(NULL);
	}
	pthread_mutex_unlock(&write_lock);

    if(VERBOSE) std::cout << my_name << ":" << side_name << ": done writing." << std::endl;
	
	return write_return_value;
}

/*--------------------------------------------------------------------------*/
/* ACCESS THE NAME OF REQUEST CHANNEL  */
/*--------------------------------------------------------------------------*/

std::string RequestChannel::name() {
	return my_name;
}

/*--------------------------------------------------------------------------*/
/* ACCESS FILE DESCRIPTORS OF REQUEST CHANNEL  */
/*--------------------------------------------------------------------------*/

int RequestChannel::read_fd() {
	return rfd;
}

int RequestChannel::write_fd() {
	return wfd;
}



