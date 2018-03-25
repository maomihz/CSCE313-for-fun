/*
    File: client.cpp

    Author: J. Higginbotham
    Department of Computer Science
    Texas A&M University
    Date  : 2016/05/21

    Based on original code by: Dr. R. Bettati, PhD
    Department of Computer Science
    Texas A&M University
    Date  : 2013/01/31
 */

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

    /* -- (none) -- */
    /* -- This might be a good place to put the size of
        of the patient response buffers -- */

#define JOHN "data John Smith"
#define JANE "data Jane Smith"
#define JOE "data Joe Smith"


/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*
    No additional includes are required
    to complete the assignment, but you're welcome to use
    any that you think would help.
*/
/*--------------------------------------------------------------------------*/

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

/*--------------------------------------------------------------------------*/
/* DATA STRUCTURES */
/*--------------------------------------------------------------------------*/

/*
    All *_params structs are optional,
    but they might help.
 */
struct PARAMS_request {
  std::string name;
  int count;
  bounded_buffer* buffer;

  PARAMS_request(bounded_buffer* buffer, int c = 0, std::string name = "")
    : name(name), count(c), buffer(buffer) {}
};

struct PARAMS_WORKER {
  bounded_buffer* buffer;
  RequestChannel* chan;
  std::unordered_map<std::string, bounded_buffer*>* responses;

  PARAMS_WORKER(bounded_buffer* buffer, RequestChannel* chan, std::unordered_map<std::string, bounded_buffer*>* responses)
    : buffer(buffer), chan(chan), responses(responses) {}
};

struct PARAMS_STAT {
  bounded_buffer* buffer;
  int count;

  PARAMS_STAT(bounded_buffer* buffer, int count)
    : buffer(buffer), count(count) {}
};

/*
    This class can be used to write to standard output
    in a multithreaded environment. It's primary purpose
    is printing debug messages while multiple threads
    are in execution.
 */
class atomic_standard_output {
    pthread_mutex_t console_lock;
public:
    atomic_standard_output() { pthread_mutex_init(&console_lock, NULL); }
    ~atomic_standard_output() { pthread_mutex_destroy(&console_lock); }
    void print(std::string s){
        pthread_mutex_lock(&console_lock);
        std::cout << s << std::endl;
        pthread_mutex_unlock(&console_lock);
    }
};

atomic_standard_output threadsafe_standard_output;

/*--------------------------------------------------------------------------*/
/* CONSTANTS */
/*--------------------------------------------------------------------------*/

    /* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* HELPER FUNCTIONS */
/*--------------------------------------------------------------------------*/

std::string make_histogram(std::string name, std::vector<int> *data) {
    std::string results = "Frequency count for " + name + ":\n";
    for(int i = 0; i < data->size(); ++i) {
        results += std::to_string(i * 10) + "-" + std::to_string((i * 10) + 9) + ": " + std::to_string(data->at(i)) + "\n";
    }
    return results;
}


// The request thread function pushes a number of the same request to the buffer.
// Usually three patient requires spawning three request threads, easy filling
// the buffer on their own.
void* request_thread_function(void* arg) {
  PARAMS_request* a = (PARAMS_request*) arg;

  for (int i = 0; i < a->count; ++i) {
    a->buffer->push_back(a->name);
  }
  return NULL;
}

// The worker thread retrieve the request from the main buffer, and sort the responses
// into another three buffers
void* worker_thread_function(void* arg) {
  PARAMS_WORKER* a = (PARAMS_WORKER*)arg;
  std::string s = a->chan->send_request("newthread");
  RequestChannel* workerChannel = new RequestChannel(s, RequestChannel::CLIENT_SIDE);

  while (true) {
    try {
      std::string request = a->buffer->retrieve_front();
      std::string response = workerChannel->send_request(request);

      // Quit
      if (request == "quit") {
        delete workerChannel;
        break;
      }
      // Sort the response
      a->responses->at(request)->push_back(response);
    }
    catch (...) {
      break;
    }
  }
  return NULL;
}


void* stat_thread_function(void* arg) {
  // Do the actual processing
  PARAMS_STAT* a = (PARAMS_STAT*)arg;

  for (int i = 0; i < a->count; ++i) {
    std::string s = a->buffer->retrieve_front();
    std::cout << s << std::endl;
  }
  return NULL;
}

/*--------------------------------------------------------------------------*/
/* MAIN FUNCTION */
/*--------------------------------------------------------------------------*/
int main(int argc, char * argv[]) {
    int n = 10; //default number of requests per "patient"
    int b = 50; //default size of request_buffer
    int w = 10; //default number of worker threads
    bool USE_ALTERNATE_FILE_OUTPUT = false;
    int opt = 0;
    while ((opt = getopt(argc, argv, "n:b:w:m:h")) != -1) {
        switch (opt) {
            case 'n':
                n = atoi(optarg);
                break;
            case 'b':
                b = atoi(optarg);
                break;
            case 'w':
                w = atoi(optarg);
                break;
            case 'm':
                if(atoi(optarg) == 2) USE_ALTERNATE_FILE_OUTPUT = true;
                break;
            case 'h':
            default:
                std::cout << "This program can be invoked with the following flags:" << std::endl;
                std::cout << "-n [int]: number of requests per patient" << std::endl;
                std::cout << "-b [int]: size of request buffer" << std::endl;
                std::cout << "-w [int]: number of worker threads" << std::endl;
                std::cout << "-m 2: use output2.txt instead of output.txt for all file output" << std::endl;
                std::cout << "-h: print this message and quit" << std::endl;
                std::cout << "Example: ./client_solution -n 10000 -b 50 -w 120 -m 2" << std::endl;
                std::cout << "If a given flag is not used, a default value will be given" << std::endl;
                std::cout << "to its corresponding variable. If an illegal option is detected," << std::endl;
                std::cout << "behavior is the same as using the -h flag." << std::endl;
                exit(0);
        }
    }

    int pid = fork();
	if (pid > 0) {
        struct timeval start_time;
        struct timeval finish_time;
        int64_t start_usecs;
        int64_t finish_usecs;
        std::ofstream ofs;
        if(USE_ALTERNATE_FILE_OUTPUT) ofs.open("output2.txt", std::ios::out | std::ios::app);
        else ofs.open("output.txt", std::ios::out | std::ios::app);

        std::cout << "n == " << n << std::endl;
        std::cout << "b == " << b << std::endl;
        std::cout << "w == " << w << std::endl;

        std::cout << "CLIENT STARTED:" << std::endl;
        std::cout << "Establishing control channel... " << std::flush;
        RequestChannel *chan = new RequestChannel("control", RequestChannel::CLIENT_SIDE);
        std::cout << "done." << std::endl;


        // Create our main buffer first!
        bounded_buffer main_buffer(b);

        // Create the response buffers
        std::unordered_map<std::string, bounded_buffer*> patients_responses;
        patients_responses["data John Smith"] = new bounded_buffer(b);
        patients_responses["data Jane Smith"] = new bounded_buffer(b);
        patients_responses["data Joe Smith" ] = new bounded_buffer(b);

        pthread_t requests[3];
        pthread_t workers[w];
        pthread_t stats[3];

        PARAMS_request user1(&main_buffer, n, "data John Smith");
        PARAMS_request user2(&main_buffer, n, "data Jane Smith");
        PARAMS_request user3(&main_buffer, n, "data Joe Smith" );
        PARAMS_WORKER  task (&main_buffer, chan, &patients_responses);
        PARAMS_STAT    stat1(patients_responses.at("data John Smith"), n);
        PARAMS_STAT    stat2(patients_responses.at("data Jane Smith"), n);
        PARAMS_STAT    stat3(patients_responses.at("data Joe Smith" ), n);

        // Spawn threads
        pthread_create(&requests[0], NULL, request_thread_function, &user1);
        pthread_create(&requests[1], NULL, request_thread_function, &user2);
        pthread_create(&requests[2], NULL, request_thread_function, &user3);

        for (int i = 0; i < w; ++i) {
          pthread_create(&workers[i], NULL, worker_thread_function, &task);
        }

        pthread_create(&stats[0], NULL, stat_thread_function, &stat1);
        pthread_create(&stats[1], NULL, stat_thread_function, &stat2);
        pthread_create(&stats[2], NULL, stat_thread_function, &stat3);


        // Join the threads and push quit
        for (int i = 0; i < 3; ++i) {
          pthread_join(requests[i], NULL);
        }

        for (int i = 0; i < w; ++i) {
          main_buffer.push_back("quit");
        }

        // Join worker threads
        for (int i = 0; i < w; ++i) {
          pthread_join(workers[i], NULL);
        }

        // Join stat threads
        for (int i = 0; i < 3; ++i) {
          pthread_join(stats[i], NULL);
        }

        ofs.close();
        std::cout << "Sleeping..." << std::endl;
        usleep(10000);
        std::string finale = chan->send_request("quit");
        std::cout << "Finale: " << finale << std::endl;

    }
	else if (pid == 0)
		execl("dataserver", NULL);
}
