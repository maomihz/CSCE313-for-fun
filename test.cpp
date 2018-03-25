#include <cassert>
#include <cstring>
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>
#include <pthread.h>
#include <string>
#include <sstream>
#include <sys/time.h>
#include <assert.h>
#include <fstream>
#include <numeric>
#include <vector>
#include <unordered_map>
#include "reqchannel.h"
/*
    This next file will need to be written from scratch, along with
    semaphore.h and (if you choose) their corresponding .cpp files.
 */

#include "bounded_buffer.h"


using namespace std;

// Trash global
semaphore s(0);
bounded_buffer buffer(5);

void* bthread_func(void* arg) {
  sleep(1);
  s.V();
  cout << "OK, I pushed." << endl;

  sleep(1);
  s.V();
  cout << "OK, I pushed again." << endl;
  return NULL;
}

void* buffer_func(void* arg) {
  while (true) {
    cout << "BUFFER: SLEEPING" << endl;
    sleep(3);
    string resp = buffer.retrieve_front();
    if (resp == "quit") {
      break;
    }
    cout << "BUFFER: " << resp << endl;
  }
  return NULL;
}

int main() {
  // Test the basic semaphore

  pthread_t test_thread;
  pthread_create(&test_thread, NULL, bthread_func, NULL);


  cout << s.val() << endl;
  s.V();
  cout << s.val() << endl;
  s.P();
  cout << s.val() << endl;

  // From now on there is not enough resources
  s.P();
  cout << s.val() << endl;
  s.P();
  cout << s.val() << endl;

  pthread_join(test_thread, NULL);


  // Now test the bounded buffer
  pthread_t buffer_thread[5];
  pthread_create(&buffer_thread[0], NULL, buffer_func, NULL);
  pthread_create(&buffer_thread[1], NULL, buffer_func, NULL);
  pthread_create(&buffer_thread[2], NULL, buffer_func, NULL);
  pthread_create(&buffer_thread[3], NULL, buffer_func, NULL);
  pthread_create(&buffer_thread[4], NULL, buffer_func, NULL);

  cout << "MAIN: Pushing 01 hello, world" << endl;
  buffer.push_back("01hello, world");
  cout << "MAIN: DONE Pushing 01 hello, world" << endl;

  cout << "MAIN: Pushing 02 hello, world" << endl;
  buffer.push_back("02hello, world");
  cout << "MAIN: DONE Pushing 02 hello, world" << endl;

  sleep(5);
  buffer.push_back("quit");
  buffer.push_back("quit");
  buffer.push_back("quit");
  buffer.push_back("quit");
  buffer.push_back("quit");

  pthread_join(buffer_thread[0], NULL);
  pthread_join(buffer_thread[1], NULL);
  pthread_join(buffer_thread[2], NULL);
  pthread_join(buffer_thread[3], NULL);
  pthread_join(buffer_thread[4], NULL);
}
