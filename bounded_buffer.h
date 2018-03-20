//
//  bounded_buffer.hpp
//  
//
//  Created by Joshua Higginbotham on 11/4/15.
//
//

#ifndef bounded_buffer_h
#define bounded_buffer_h

#include <stdio.h>
#include <queue>
#include <string>
#include <pthread.h>
#include "semaphore.h"

class bounded_buffer {
	/* Internal data here */
public:
    bounded_buffer(int _capacity);
    void push_back(std::string str);
    std::string retrieve_front();
    int size();
};

#endif /* bounded_buffer_h */
