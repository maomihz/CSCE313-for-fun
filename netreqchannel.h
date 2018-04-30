#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <pthread.h>
#include <netdb.h>
#include <string.h>
#include <string>
#include <stdlib.h>
#include <unistd.h>


class NetworkRequestChannel {
public:
  NetworkRequestChannel(const std::string _server_host_name, const std::string _port_no);

  NetworkRequestChannel(const std::string _port_no, void* (*ch) (void*), int backlog_buffer = 20);

  NetworkRequestChannel(int sfd);

  ~NetworkRequestChannel();

  std::string send_request(std::string _request);
  std::string cread();
  int cwrite(std::string _msg);

  int socket_fd() {
    return sockfd;
  }
private:
  int sockfd;
};
