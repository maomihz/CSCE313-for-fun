#include <string>

using namespace std;

struct client_info {
    string data;
    SafeBuffer* buffer;
    int n;      // Number of requests to push
};

struct worker_info {
    SafeBuffer* buffer;
    RequestChannel* chan;
    Histogram* hist;
};