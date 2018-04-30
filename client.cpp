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
#include <time.h>
#include <signal.h>
#include <iomanip>
#include <stdexcept>
#include <sys/select.h>
#include "netreqchannel.h"
/*
    This next file will need to be written from scratch, along with
    semaphore.h and (if you choose) their corresponding .cpp files.
 */

#include "bounded_buffer.h"
#include "safecounter.h"

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
  int w;
  bounded_buffer* buffer;
  std::string host, port;
  std::unordered_map<std::string, bounded_buffer*>* responses;


  PARAMS_WORKER(int w, bounded_buffer* buffer, std::string host, std::string port,
    std::unordered_map<std::string, bounded_buffer*>* responses)
    : w(w), buffer(buffer), host(host), port(port), responses(responses) {}
};

struct PARAMS_STAT {
  bounded_buffer* buffer;
  std::string name;
  int count;
  SafeCounter* counter;

  PARAMS_STAT(bounded_buffer* buffer, std::string name, int count, SafeCounter* counter)
    : buffer(buffer), name(name), count(count), counter(counter) {}
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
  // Initialize arguments
  PARAMS_WORKER* a = (PARAMS_WORKER*)arg;
  int w = a->w;

  // Setup basic structures
  std::string requests[w];
  NetworkRequestChannel* workerChannels[w];

  // setup select() structures
  fd_set fds;
  int max_fd = -1;


  // Initialize request channels
  FD_ZERO(&fds);
  for (int i = 0; i < w; ++i) {
    workerChannels[i] = new NetworkRequestChannel(a->host, a->port);
    int fd = workerChannels[i]->socket_fd();
    if (fd > max_fd) {
      max_fd = fd;
    }

    std::string request = a->buffer->retrieve_front();
    requests[i] = request;
    workerChannels[i]->cwrite(request);

    // Add to the descriptor set
    FD_SET(fd, &fds);
  }


  // Initialize fd set
  while (true) {
    int sel = select(max_fd + 1, &fds, NULL, NULL, 0);

    if (sel == -1) {
      throw std::runtime_error("Something is wrong with select()");
    } else if (!sel) { // Timeout
      continue;
    }


    for (int i = 0; i < w; ++i) {
      if (workerChannels[i] == NULL) {
        continue;
      }
      if (FD_ISSET(workerChannels[i]->socket_fd(), &fds)) {
        std::string response = workerChannels[i]->cread();
        if (requests[i] == "quit") {
          delete workerChannels[i];
          workerChannels[i] = NULL;
          continue;
        }
        a->responses->at(requests[i])->push_back(response);

        // Send request again
        std::string request = a->buffer->retrieve_front();
        requests[i] = request;
        workerChannels[i]->cwrite(request);

      }
    }

    // Re-initialize fds
    FD_ZERO(&fds);
    max_fd = -1;
    for (int i = 0; i < w; ++i) {
      if (workerChannels[i] == NULL) {
        continue;
      }
      int fd = workerChannels[i]->socket_fd();
      FD_SET(fd, &fds);
      if (fd > max_fd) {
        max_fd = fd;
      }
    }
    if (max_fd < 0) {
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
    // std::cout << "Response: "<< s << std::endl;
    a->counter->inc(a->name, stoi(s) / 10);
  }
  return NULL;
}

void livedisplay_handler (int signo, siginfo_t* info, void* context)
{
  static int count = 0;
  SafeCounter* counter = (SafeCounter*)(info->si_value.sival_ptr);

  std::vector<int> john_frequency_count(10, 0);
  std::vector<int> jane_frequency_count(10, 0);
  std::vector<int> joe_frequency_count(10, 0);
  for (int i = 0; i < 10; ++i) {
    john_frequency_count.at(i) = counter->count("John", i);
    jane_frequency_count.at(i) = counter->count("Jane", i);
    joe_frequency_count.at(i) = counter->count("Joe", i);
  }
  std::string histogram_table = make_histogram_table("John Smith",
      "Jane Smith", "Joe Smith", &john_frequency_count,
      &jane_frequency_count, &joe_frequency_count);
   printf( "\033[;H" );
  std::cout << histogram_table << std::endl;

 // std::cout << make_histogram("John", &john_frequency_count) << std::endl;
 // std::cout << make_histogram("Jane", &jane_frequency_count) << std::endl;
 // std::cout << make_histogram("Joe", &joe_frequency_count) << std::endl;
}

/*--------------------------------------------------------------------------*/
/* MAIN FUNCTION */
/*--------------------------------------------------------------------------*/
int main(int argc, char * argv[]) {
    int n = 10; //default number of requests per "patient"
    int b = 50; //default size of request_buffer
    int w = 10; //default number of worker threads
    std::string server_host;
    std::string server_port;
    bool USE_ALTERNATE_FILE_OUTPUT = false;
    int opt = 0;
    while ((opt = getopt(argc, argv, "n:b:w:h:p:m")) != -1) {
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
            case 'h':
                server_host = optarg;
                break;
            case 'p':
                server_port = optarg;
                break;
            case 'm':
                if(atoi(optarg) == 2) USE_ALTERNATE_FILE_OUTPUT = true;
                break;
            // case 'h':
            default:
                std::cout << "This program can be invoked with the following flags:" << std::endl;
                std::cout << "-n [int]: number of requests per patient" << std::endl;
                std::cout << "-b [int]: size of request buffer" << std::endl;
                std::cout << "-w [int]: number of worker threads" << std::endl;
                std::cout << "-h [string]: name of server host" << std::endl;
                std::cout << "-p [int]: port number of server host" << std::endl;
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
        std::cout << "No need to establishing control channel... " << std::flush;
        // NetworkRequestChannel *chan = new NetworkRequestChannel(server_host, server_port);
        std::cout << "Not done." << std::endl;


        // Create our main buffer first!
        bounded_buffer main_buffer(b);
        SafeCounter response_counter;

        // Create the response buffers
        std::unordered_map<std::string, bounded_buffer*> patients_responses;
        patients_responses["data John Smith"] = new bounded_buffer(b);
        patients_responses["data Jane Smith"] = new bounded_buffer(b);
        patients_responses["data Joe Smith" ] = new bounded_buffer(b);

        // Create the threads
        pthread_t requests[3];
        pthread_t worker;
        pthread_t stats[3];

        // Create the data structures
        PARAMS_request user1(&main_buffer, n, "data John Smith");
        PARAMS_request user2(&main_buffer, n, "data Jane Smith");
        PARAMS_request user3(&main_buffer, n, "data Joe Smith" );
        PARAMS_WORKER  task (w, &main_buffer, server_host, server_port, &patients_responses);
        PARAMS_STAT    stat1(patients_responses.at("data John Smith"), "John", n, &response_counter);
        PARAMS_STAT    stat2(patients_responses.at("data Jane Smith"), "Jane", n, &response_counter);
        PARAMS_STAT    stat3(patients_responses.at("data Joe Smith" ), "Joe" , n, &response_counter);

        // Spawn threads
        pthread_create(&requests[0], NULL, request_thread_function, &user1);
        pthread_create(&requests[1], NULL, request_thread_function, &user2);
        pthread_create(&requests[2], NULL, request_thread_function, &user3);

        pthread_create(&worker, NULL, worker_thread_function, &task);

        pthread_create(&stats[0], NULL, stat_thread_function, &stat1);
        pthread_create(&stats[1], NULL, stat_thread_function, &stat2);
        pthread_create(&stats[2], NULL, stat_thread_function, &stat3);



        // Start the live display
        struct sigaction sa;
        struct sigevent sev;
        timer_t timerid;

        sa.sa_flags = SA_SIGINFO;
        sa.sa_sigaction = &livedisplay_handler;
        sigaction (SIGALRM, &sa, NULL);

        sev.sigev_value.sival_int = 123;
        sev.sigev_value.sival_ptr = &response_counter;
        sev.sigev_notify = SIGEV_SIGNAL;
        sev.sigev_notify_attributes = NULL;
        sev.sigev_signo = SIGALRM;

        timer_create(CLOCK_REALTIME, &sev, &timerid);

        struct itimerspec value;
        value.it_interval.tv_sec = 1;
        value.it_interval.tv_nsec = 0;
        value.it_value.tv_sec = 1;
        value.it_value.tv_nsec = 0;

        timer_settime(timerid, 0, &value, NULL);

        // Lots of work
        printf("\033[2J");


        // Start timer!!
        gettimeofday(&start_time, NULL);


        // Join the threads and push quit
        for (int i = 0; i < 3; ++i) {
          pthread_join(requests[i], NULL);
        }

        // Push the quit requests
        for (int i = 0; i < w; ++i) {
          main_buffer.push_back("quit");
        }

        // Join worker threads
        pthread_join(worker, NULL);

        // // Join stat threads
        for (int i = 0; i < 3; ++i) {
          pthread_join(stats[i], NULL);
        }

        timer_delete(timerid);
        // End timer!!
        gettimeofday(&finish_time, NULL);
        start_usecs = start_time.tv_sec * 1000000L + start_time.tv_usec;
        finish_usecs = finish_time.tv_sec * 1000000L + finish_time.tv_usec;

        // End
        delete patients_responses["data John Smith"];
        delete patients_responses["data Jane Smith"];
        delete patients_responses["data Joe Smith" ];


        // Draw histogram
        std::vector<int> john_frequency_count(10, 0);
        std::vector<int> jane_frequency_count(10, 0);
        std::vector<int> joe_frequency_count(10, 0);
        for (int i = 0; i < 10; ++i) {
          john_frequency_count.at(i) = response_counter.count("John", i);
          jane_frequency_count.at(i) = response_counter.count("Jane", i);
          joe_frequency_count.at(i) = response_counter.count("Joe", i);
        }

        std::cout << make_histogram("John", &john_frequency_count) << std::endl;
        std::cout << make_histogram("Jane", &jane_frequency_count) << std::endl;
        std::cout << make_histogram("Joe", &joe_frequency_count) << std::endl;

        ofs.close();
        std::cout << "Sleeping..." << std::endl;
        usleep(10000);
        // std::string finale = chan->send_request("quit");
        // std::cout << "No Finale: " << finale << std::endl;

        std::cout << "Running time: " << finish_usecs - start_usecs << std::endl;



    }
	// else if (pid == 0)
	// 	execl("dataserver -p 9999", NULL);
}
