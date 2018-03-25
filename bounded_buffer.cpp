//
//  bounded_buffer.cpp
//
//
//  Created by Joshua Higginbotham on 11/4/15.
//
//

#include "bounded_buffer.h"
#include <string>
#include <queue>

bounded_buffer::bounded_buffer(int _capacity)
: buffer(), full(0), empty(_capacity) {
  pthread_mutex_init(&mutex, NULL);
}

void bounded_buffer::push_back(std::string req) {
  empty.P();

  pthread_mutex_lock(&mutex);
  buffer.push(req);
  pthread_mutex_unlock(&mutex);

  full.V();
}

std::string bounded_buffer::retrieve_front() {
  full.P();

  pthread_mutex_lock(&mutex);
  std::string item = buffer.front();
  buffer.pop();
  pthread_mutex_unlock(&mutex);

  empty.V();
  return item;
}

int bounded_buffer::size() {
  pthread_mutex_lock(&mutex);
  size_t size = buffer.size();
  pthread_mutex_unlock(&mutex);
  return size;
}
