#include <string>
#include <pthread.h>
#include <unordered_map>


class SafeCounter {
  /*
    SafeCounter is a wrapper around unordered_list. Counts frequency of
    a string.
  */

public:
  SafeCounter();
  ~SafeCounter();
  void inc(std::string s);
  void inc(std::string s, int num);
  unsigned int count(std::string s);
  unsigned int count(std::string s, int num);
private:
  pthread_mutex_t mtx;
  std::unordered_map<std::string, unsigned int> cmap;
  std::string transform(std::string s, int num);
};
