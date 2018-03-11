//
//  SafeBuffer.cpp
//
//
//  Created by Joshua Higginbotham on 11/4/15.
//
//

#include "SafeBuffer.h"
#include <string>
#include <queue>
#include <utility>

SafeBuffer::SafeBuffer() : vec() {
  pthread_mutex_init(&mtx, NULL);
}

SafeBuffer::~SafeBuffer() {
  pthread_mutex_destroy(&mtx);
}

int SafeBuffer::size() {
    return vec.size();
}

void SafeBuffer::push_back(std::string str) {
  pthread_mutex_lock(&mtx);
  vec.push(str);
  pthread_mutex_unlock(&mtx);
  return;
}

std::string SafeBuffer::retrieve_front() {
  pthread_mutex_lock(&mtx);
  std::string s = vec.front();
  vec.pop();
  pthread_mutex_unlock(&mtx);
  return s;
}


SafeCounter::SafeCounter() : cmap() {
  pthread_mutex_init(&mtx, NULL);
}

SafeCounter::~SafeCounter() {
  pthread_mutex_destroy(&mtx);
}

void SafeCounter::inc(std::string s) {
  pthread_mutex_lock(&mtx);
  try {
    cmap.at(s) ++;
    pthread_mutex_unlock(&mtx);
  } catch (std::out_of_range e) {
    cmap[s] = 1;
    pthread_mutex_unlock(&mtx);
  }
}

void SafeCounter::inc(std::string s, int num) {
  inc(transform(s, num));
}

unsigned int SafeCounter::count(std::string s) {
  try {
    return cmap.at(s);
  } catch (std::out_of_range e) {
    return 0;
  }
}

unsigned int SafeCounter::count(std::string s, int num) {
  return count(transform(s, num));
}

std::string SafeCounter::transform(std::string s, int num) {
  return s + "$$" + std::to_string(num);
}


// unsigned int
