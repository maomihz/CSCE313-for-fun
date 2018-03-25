#include "safecounter.h"


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
