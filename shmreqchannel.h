#include <iostream>
#include <sstream>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include "kernelsemaphore.h"
#define SHM_SIZE 1024

class RequestChannel {
public:
  typedef enum {SERVER_SIDE, CLIENT_SIDE} Side;
  typedef enum {READ_MODE, WRITE_MODE} Mode;

  RequestChannel(const std::string name, const Side side);
  ~RequestChannel();

  std::string send_request(std::string request);

  // Return the # characters
  int cwrite(std::string msg);
  std::string cread();
  std::string name();
private:
  key_t rkey, wkey, rkey2, wkey2;
  int rshmid, wshmid;
  KernelSemaphore *rsem_full, *wsem_full, *rsem_empty, *wsem_empty;
};
