#include "SafeBuffer.h"

SafeBuffer::SafeBuffer() {
    pthread_mutex_init(&mutex, NULL);
}

SafeBuffer::~SafeBuffer() {
    pthread_mutex_destroy(&mutex);
}

int SafeBuffer::size() {
    pthread_mutex_lock(&mutex);
    size_t queue_size = q.size();
    pthread_mutex_unlock(&mutex);
    return queue_size;
}

void SafeBuffer::push(string str) {
    pthread_mutex_lock(&mutex);
    q.push (str);
    pthread_mutex_unlock(&mutex);
}

string SafeBuffer::pop() {
    pthread_mutex_lock(&mutex);
    string s = q.front();
    q.pop();
    pthread_mutex_unlock(&mutex);
    return s;
}
