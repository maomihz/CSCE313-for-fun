//
//  SafeBuffer.h
//  
//
//  Created by Joshua Higginbotham on 11/4/15.
//
//

#ifndef SafeBuffer_h
#define SafeBuffer_h

#include <stdio.h>
#include <queue>
#include <string>
#include <pthread.h>

class SafeBuffer {
	/*
		Only two data members are really necessary for this class:
		a mutex, and a data structure to hold elements. Recall
		that SafeBuffer is supposed to be FIFO, so something
		like std::vector will adversely affect performance if
		used here. We recommend something like std::list
		or std::queue, because std::vector is very inefficient when
		being modified from the front.
	*/
	
public:
    SafeBuffer();
	~SafeBuffer();
	int size();
    void push_back(std::string str);
    std::string retrieve_front();
};

#endif /* SafeBuffer_ */
