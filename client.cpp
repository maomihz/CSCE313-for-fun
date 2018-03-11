/*
  File: client_MP6.cpp

  Author: J. Higginbotham
  Department of Computer Science
  Texas A&M University
  Date  : 2016/05/20
  Last Modified : 2017/03/20


  Based on original assignment by: Dr. R. Bettati, PhD
  Department of Computer Science
  Texas A&M University
  Date  : 2013/01/31
 */

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*
  This assignment does not require any additional includes
   besides un-commenting "SafeBuffer.h", nor will you probably use
   all the provided includes (I can't even remember if the solution
  code uses all these includes...), but you are free to add any that
   you find helpful.
*/
/*--------------------------------------------------------------------------*/

#include <iostream>
#include <fstream>
#include <cstring>
#include <string>
#include <sstream>
#include <iomanip>

#include <sys/time.h>
#include <cassert>
#include <assert.h>

#include <cmath>
#include <numeric>
#include <algorithm>

#include <list>
#include <vector>

#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>
#include <pthread.h>

#include "reqchannel.h"
#include "SafeBuffer.h"

/*
  You are allowed to add a namespace declaration if you
  think it will help, although that is discouraged as being
  a poor voding practice.

  You are also allowed to remove any/all comments if you find it
  makes your code more readable, but still pay attention in case
  the comment contains important/helpful information about the
  assignment.
*/

/*--------------------------------------------------------------------------*/
/* DATA STRUCTURES */
/*--------------------------------------------------------------------------*/

/*
 * This is a good place to write in storage structs
 * with which to pass parameters to the worker
 * and request thread functions, as well as any other
 * classes you think will be helpful.
 */

struct user_info {
  std::string name;
  int count;
  SafeBuffer* buffer;
  user_info(SafeBuffer* b, int c = 0, std::string name = "")
    : name(name)
    , count(c)
    , buffer(b)
  {
  }
};

struct task_info {
  SafeBuffer* buffer;
  SafeCounter* count;
  RequestChannel* chan;
  task_info(SafeBuffer* b, SafeCounter* c, RequestChannel* chan)
    : buffer(b)
    , count(c)
    , chan(chan)
  {
  }
};
/*
  The class below allows any thread in this program to use
  the variable "atomic_standard_output" down below to write output to the
  console without it getting jumbled together with the output
  from other threads.
  For example:
  thread A {
    atomic_standard_output.println("Hello ");
  }
  thread B {
    atomic_standard_output.println("World!\n");
  }
  The output could be:

  Hello
  World!

  or...

  World!

  Hello

  ...but it will NOT be something like:

  HelWorllo
  d!

 */

class atomic_standard_output {
  /*
     Note that this class provides an example
     of the usage of mutexes, which are a crucial
     synchronization type. You will probably not
     be able to complete this assignment without
  using mutexes.
   */
  pthread_mutex_t console_lock;

public:
  atomic_standard_output()
  {
    pthread_mutex_init(&console_lock, NULL);
  }
  ~atomic_standard_output()
  {
    pthread_mutex_destroy(&console_lock);
  }
  void println(std::string s)
  {
    pthread_mutex_lock(&console_lock);
    std::cout << s << std::endl;
    pthread_mutex_unlock(&console_lock);
  }
  void perror(std::string s)
  {
    pthread_mutex_lock(&console_lock);
    std::cerr << s << ": " << strerror(errno) << std::endl;
    pthread_mutex_unlock(&console_lock);
  }
};

atomic_standard_output threadsafe_console_output;

/*--------------------------------------------------------------------------*/
/* HELPER FUNCTIONS */
/*
  You may add any helper functions you like, or change the provided functions,
  so long as your solution ultimately conforms to what is explicitly required
  by the handout.
*/
/*--------------------------------------------------------------------------*/

/*
  This function is provided for your convenience. If you vhoose not to use it,
  your final display should still be a histogram with bin size 10.
*/
std::string make_histogram_table(std::string name1, std::string name2,
  std::string name3, std::vector<int>* data1, std::vector<int>* data2,
  std::vector<int>* data3)
{
  std::stringstream tablebuilder;
  tablebuilder << std::setw(25) << std::right << name1;
  tablebuilder << std::setw(15) << std::right << name2;
  tablebuilder << std::setw(15) << std::right << name3 << std::endl;
  for (int i = 0; i < data1->size(); ++i) {
    tablebuilder << std::setw(10) << std::left
           << std::string(
              std::to_string(i * 10) + "-"
              + std::to_string((i * 10) + 9));
    tablebuilder << std::setw(15) << std::right
           << std::to_string(data1->at(i));
    tablebuilder << std::setw(15) << std::right
           << std::to_string(data2->at(i));
    tablebuilder << std::setw(15) << std::right
           << std::to_string(data3->at(i)) << std::endl;
  }
  tablebuilder << std::setw(10) << std::left << "Total";
  tablebuilder << std::setw(15) << std::right
         << accumulate(data1->begin(), data1->end(), 0);
  tablebuilder << std::setw(15) << std::right
         << accumulate(data2->begin(), data2->end(), 0);
  tablebuilder << std::setw(15) << std::right
         << accumulate(data3->begin(), data3->end(), 0) << std::endl;

  return tablebuilder.str();
}

void* request_thread_function(void* arg)
{
  user_info* u = (user_info*)arg;

  for (int i = 0; i < u->count; ++i) {
    u->buffer->push_back(u->name);
  }
  return NULL;
}

void* worker_thread_function(void* arg)
{
  task_info* t = (task_info*)arg;
  std::string s = t->chan->send_request("newthread");
  RequestChannel* workerChannel = new RequestChannel(s, RequestChannel::CLIENT_SIDE);

  while (true) {
    try {
      std::string request = t->buffer->retrieve_front();
      std::string response = workerChannel->send_request(request);

      if (request == "data John Smith") {
        t->count->inc("John", stoi(response) / 10);
      }
      else if (request == "data Jane Smith") {
        t->count->inc("Jane", stoi(response) / 10);
      }
      else if (request == "data Joe Smith") {
        t->count->inc("Joe", stoi(response) / 10);
      }
      else if (request == "quit") {
        delete workerChannel;
        break;
      }
    }
    catch (...) {
      break;
    }
  }
  return NULL;
}

/*--------------------------------------------------------------------------*/
/* MAIN FUNCTION */
/*--------------------------------------------------------------------------*/

int main(int argc, char* argv[])
{
  /*
    Note that the assignment calls for n = 10000.
    That is far too large for a single thread to accomplish quickly.
  */
  int n = 100; //default number of requests per "patient"
  int w = 3;   //default number of worker threads
  int opt = 0;
  while ((opt = getopt(argc, argv, "n:w:h")) != -1) {
    switch (opt) {
    case 'n':
      n = atoi(optarg);
      break;
    case 'w':
      w = atoi(optarg); //This won't do a whole lot until you fill in the worker thread function
      break;
    case 'h':
    default:
      std::cout << "This program can be invoked with the following flags:" << std::endl;
      std::cout << "-n [int]: number of requests per patient" << std::endl;
      std::cout << "-w [int]: number of worker threads" << std::endl;
      std::cout << "-h: print this message and quit" << std::endl;
      std::cout << "(Canonical) example: ./client_solution -n 10000 -w 128" << std::endl;
      std::cout << "If a given flag is not used, or given an invalid value," << std::endl;
      std::cout << "a default value will be given to the corresponding variable." << std::endl;
      std::cout << "If an illegal option is detected, behavior is the same as using the -h flag." << std::endl;
      exit(0);
    }
  }

  int pid = fork();
  if (pid > 0) {
    std::cout << "n == " << n << std::endl;
    std::cout << "w == " << w << std::endl;

    std::cout << "CLIENT STARTED:" << std::endl;
    std::cout << "Establishing control channel... " << std::flush;
    RequestChannel* chan = new RequestChannel("control", RequestChannel::CLIENT_SIDE);
    std::cout << "done." << std::endl;

    /*
      All worker threads will use the following structures,
      but the current implementation puts
      them on the stack in main. You can change the code
      to use the heap if you'd like, or even a
      container other than std::vector. However,
      in the end you MUST use SafeBuffer, or some other
      threadsafe, FIFO data structure, instead
      of std::list for the request_buffer.
     */

    SafeBuffer request_buffer;
    SafeCounter response_counter;

/*--------------------------------------------------------------------------*/
    /*  BEGIN CRITICAL SECTION  */
    /*
      You will modify the program so that client_MP6.cpp
      populates the request buffer using 3 request threads
      instead of sequentially in a loop, and likewise
      processes requests using w worker threads instead of
      sequentially in a loop.

      Note that in the finished product, as in this initial code,
      all the requests will be pushed to the request buffer
      BEFORE the worker threads begin processing them. This means
      that you will have to ensure that all 3 request threads
      have terminated before kicking off any worker threads.
      In the next machine problem, we will deal with the
      synchronization problems that arise from allowing the
      request threads and worker threads to run in parallel
      with each other. In the meantime, see if you can figure
      out the limitations of this machine problem's approach (*cough*stack overflow,
      as in not the Q&A site*coughcough*).

      Just to be clear: for this machine problem, request
      threads will run concurrently with request threads, and worker
      threads will run concurrently with worker threads, but request
      threads will NOT run concurrently with worker threads.

      While gathering data for your report, it is recommended that you comment
      out all the output operations occurring between when you
      start the timer and when you end the timer. Output operations
      are very time-intensive (at least compared to the other operations
      we're doing) and will skew your results.
    */
/*--------------------------------------------------------------------------*/

    std::cout << "Populating request buffer... ";
    fflush(NULL);

    pthread_t client_thread1;
    pthread_t client_thread2;
    pthread_t client_thread3;

    user_info user1(&request_buffer, n, "data John Smith");
    user_info user2(&request_buffer, n, "data Jane Smith");
    user_info user3(&request_buffer, n, "data Joe Smith");

    pthread_create(&client_thread1, NULL, request_thread_function, &user1);
    pthread_create(&client_thread2, NULL, request_thread_function, &user2);
    pthread_create(&client_thread3, NULL, request_thread_function, &user3);

    pthread_join(client_thread1, NULL);
    pthread_join(client_thread2, NULL);
    pthread_join(client_thread3, NULL);
    std::cout << "done." << std::endl;

    std::cout << "Pushing quit requests... ";
    fflush(NULL);
    for (int i = 0; i < w; ++i) {
      request_buffer.push_back("quit");
    }
    std::cout << "done." << std::endl;

    /*-------------------------------------------*/
    /* START TIMER HERE */
    /*-------------------------------------------*/
    timespec start_time, stop_time;
    clock_gettime(CLOCK_REALTIME, &start_time);


    pthread_t workers[w];
    task_info task(&request_buffer, &response_counter, chan);

    for (int i = 0; i < w; ++i) {
      pthread_create(&workers[i], NULL, worker_thread_function, &task);
    }
    for (int i = 0; i < w; ++i) {
      pthread_join(workers[i], NULL);
    }



    /*--------------------------------------------------------------------------*/
    /*  END CRITICAL SECTION  */
    /*--------------------------------------------------------------------------*/

    /*
      By the point at which you end the timer,
      all worker threads should have terminated.
      Note that the containers from earlier (namely
      request_buffery and the *frequency_count vectors)
      are still in scope, so threads can still use them
      if they have a pointer to them.
     */

    /*-------------------------------------------*/
    /* END TIMER HERE   */
    /*-------------------------------------------*/
    clock_gettime(CLOCK_REALTIME, &stop_time);
    long long time_usec = (stop_time.tv_nsec - start_time.tv_nsec) / 1000L
      + (stop_time.tv_sec - start_time.tv_sec) * 1000000L;

    /*
      You may want to eventually add file output
      to this section of the code in order to make it easier
      to assemble the timing data from different iterations
      of the program. If you do, (and this is just a recommendation)
      try to use a format that will be easy for you to make a graph with later,
      such as comma-separated values (LaTeX) or labeled rows and columns
      (spreadsheet software like Excel and Google Sheets).
     */

    std::cout << "Finished!" << std::endl;

    std::vector<int> john_frequency_count(10, 0);
    std::vector<int> jane_frequency_count(10, 0);
    std::vector<int> joe_frequency_count(10, 0);

    for (int i = 0; i < 10; ++i) {
      john_frequency_count.at(i) = response_counter.count("John", i);
      jane_frequency_count.at(i) = response_counter.count("Jane", i);
      joe_frequency_count.at(i) = response_counter.count("Joe", i);
    }

    std::string histogram_table = make_histogram_table("John Smith",
      "Jane Smith", "Joe Smith", &john_frequency_count,
      &jane_frequency_count, &joe_frequency_count);

    std::cout << "Results for n == " << n << ", w == " << w << std::endl;

    /*
     This is a good place to output your timing data.
    */

    std::cout << histogram_table << std::endl;
    std::cout << "Sleeping..." << std::endl;
    usleep(10000);

    /*
      EVERY RequestChannel must send a "quit"
      request before program termination, and
      the destructor for each RequestChannel must be
      called somehow. If your RequestChannels are allocated
      on the stack, they will be destroyed automatically.
      If on the heap, you must properly free it. Take
      this seriously, it is part of your grade.
    */
    std::string finale = chan->send_request("quit");
    delete chan;
    std::cout << "Finale: " << finale << std::endl; //This line, however, is optional.
    std::cout << "Running Time: " << time_usec << " (microseconds)" << std::endl;
  }
  else if (pid == 0)
    execl("dataserver", (char*)NULL);
}
