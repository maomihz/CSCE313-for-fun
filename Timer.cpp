#include "Timer.h"


Timer::Timer(bool start) {
    if (start) {
        this->start();
    }
}

void Timer::start() {
    gettimeofday(&tp_start, 0);
}

void Timer::stop() {
    gettimeofday(&tp_end, 0);
}

long Timer::sec() {
    long sec = tp_end.tv_sec - tp_start.tv_sec;
    long musec = tp_end.tv_usec - tp_start.tv_usec;
    if (musec < 0) {
        return sec - 1;
    }
    return sec;
}

long Timer::musec() {
    long musec = tp_end.tv_usec - tp_start.tv_usec;
    if (musec < 0) {
        return musec + (int)1e6;
    }
    return musec;
}

ostream& operator<<(ostream& os, Timer& t) {
    os << "[sec = "<< t.sec();
    os << ", musec = "<< t.musec();
    os << "]";
    return os;
}