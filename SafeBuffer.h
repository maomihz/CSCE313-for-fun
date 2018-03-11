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
#include <list>
#include <pthread.h>
#include <unordered_map>

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
private:
	pthread_mutex_t mtx;
  std::list<std::string> vec;
};


class SafeCounter {
  /*
    SafeCounter is a wrapper around unordered_list. Counts frequency of
    a string.
  */

public:
  SafeCounter();
  ~SafeCounter();
  void inc(std::string s);
  void inc(std::string s, int num);
  unsigned int count(std::string s);
  unsigned int count(std::string s, int num);
private:
  pthread_mutex_t mtx;
  std::unordered_map<std::string, unsigned int> cmap;
  std::string transform(std::string s, int num);
};

#endif /* SafeBuffer_ */
