#include <iostream>
#include <sys/time.h>

using namespace std;

// Use Timer class to easily keep track of running time
class Timer {
private:
    struct timeval tp_start;
    struct timeval tp_end;

public:
    Timer(bool start = false);
    void start();
    void stop();
    long sec();
    long musec();
};

ostream& operator<<(ostream& os, Timer& t);