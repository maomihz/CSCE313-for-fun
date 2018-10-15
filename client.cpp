/*
    Based on original assignment by: Dr. R. Bettati, PhD
    Department of Computer Science
    Texas A&M University
    Date  : 2013/01/31
 */


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
#include "Histogram.h"
#include "client.h"
using namespace std;


void* request_thread_function(void* arg) {
    client_info* client = (client_info*)arg;
    /*
        Fill in this function.

        The loop body should require only a single line of code.
        The loop conditions should be somewhat intuitive.

        In both thread functions, the arg parameter
        will be used to pass parameters to the function.
        One of the parameters for the request thread
        function MUST be the name of the "patient" for whom
        the data requests are being pushed: you MAY NOT
        create 3 copies of this function, one for each "patient".
     */

    for(int i = 0; i < client->n; i++) {
        client->buffer->push(client->data);
    }
}

void* worker_thread_function(void* arg) {
    worker_info* worker = (worker_info*)arg;
    /*
        Fill in this function. 

        Make sure it terminates only when, and not before,
        all the requests have been processed.

        Each thread must have its own dedicated
        RequestChannel. Make sure that if you
        construct a RequestChannel (or any object)
        using "new" that you "delete" it properly,
        and that you send a "quit" request for every
        RequestChannel you construct regardless of
        whether you used "new" for it.
     */

    while(true) {
        string request = worker->buffer->pop();
        worker->chan->cwrite(request);

        if(request == "quit") {
            delete worker->chan;
            break;
        } else {
            string response = worker->chan->cread();
            worker->hist->update (request, response);
        }

    }
}

/*--------------------------------------------------------------------------*/
/* MAIN FUNCTION */
/*--------------------------------------------------------------------------*/

int main(int argc, char * argv[]) {
    int n = 100; //default number of requests per "patient"
    int w = 1; //default number of worker threads
    int opt = 0;
    while ((opt = getopt(argc, argv, "n:w:")) != -1) {
        switch (opt) {
            case 'n':
                n = atoi(optarg);
                break;
            case 'w':
                w = atoi(optarg); //This won't do a whole lot until you fill in the worker thread function
                break;
            }
    }

    int pid = fork();
    if (pid == 0){
        execl("dataserver", (char*) NULL);
    }
    else {
        // Print initial info, create control channel and other data structures
        cout << "n == " << n << endl;
        cout << "w == " << w << endl;

        cout << "CLIENT STARTED:" << endl;
        cout << "Establishing control channel... " << flush;
        RequestChannel *chan = new RequestChannel("control", RequestChannel::CLIENT_SIDE);
        cout << "done." << endl<< flush;

        SafeBuffer request_buffer;
        Histogram hist;
        struct timeval tp_start, tp_end;
        // Start timer!
        gettimeofday(&tp_start, 0);

        // Populate requests into the buffer
        const int CLIENT_COUNT = 3;
        client_info request_clients[CLIENT_COUNT];
        request_clients[0].data = "data John Smith";
        request_clients[1].data = "data Jane Smith";
        request_clients[2].data = "data Joe Smith";

        // Create request threads
        pthread_t request_threads[CLIENT_COUNT];

        for (int i = 0; i < CLIENT_COUNT; i++) {
            request_clients[i].buffer = &request_buffer;
            request_clients[i].n = n;
            pthread_create(&request_threads[i], NULL, request_thread_function, &request_clients[i]);
        }
        for (int i = 0; i < CLIENT_COUNT; i++) {
            pthread_join(request_threads[i], NULL);
        }
        cout << "Done populating request buffer" << endl;

        cout << "Pushing quit requests... ";
        for(int i = 0; i < w; ++i) {
            request_buffer.push("quit");
        }
        cout << "done." << endl;


        // Create workers
        worker_info worker_clients[w];
        pthread_t worker_threads[w];

        for (int i = 0; i < w; i++) {
            worker_clients[i].buffer = &request_buffer;
            worker_clients[i].hist = &hist;

            // Create a new channel
            chan->cwrite("newchannel");
            string s = chan->cread();
            worker_clients[i].chan = new RequestChannel(s, RequestChannel::CLIENT_SIDE);

            pthread_create(&worker_threads[i], NULL, worker_thread_function, &worker_clients[i]);
        }
        for (int i = 0; i < w; i++) {
            pthread_join(worker_threads[i], NULL);
        }


        // Delete control channels
        chan->cwrite ("quit");
        delete chan;
        cout << "All Done!!!" << endl; 
        // End timer!
        gettimeofday(&tp_end, 0);

        hist.print ();

        // Compute running time
        long sec = tp_end.tv_sec - tp_start.tv_sec;
        long musec = tp_end.tv_usec - tp_start.tv_usec;
        if (musec < 0) {
            musec += (int)1e6;
            sec--;
        }
        stringstream ss;
        cout << "Time taken: [sec = "<< sec <<", musec = "<<musec<< "]" << endl;
    }
}
