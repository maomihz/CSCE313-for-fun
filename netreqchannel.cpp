#include "netreqchannel.h"

// Client side constructor
NetworkRequestChannel::NetworkRequestChannel(const std::string _server_host_name, const std::string _port_no) {
  struct addrinfo hints, *res;

  // first, load up address structs with getaddrinfo():
  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  int status;
  if ((status = getaddrinfo(_server_host_name.c_str(), _port_no.c_str(), &hints, &res)) != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
    exit(-1);
  }

  // make a socket:
  sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
  if (sockfd < 0)
  {
    perror ("Error creating socket\n");
    exit(-1);
  }

  // connect!
  if (connect(sockfd, res->ai_addr, res->ai_addrlen)<0)
  {
    perror ("connect error\n");
    exit(-1);
  }
}

// Server side constuctor
NetworkRequestChannel::NetworkRequestChannel(const std::string _port_no, void* (*ch) (void*), int backlog_buffer) {
  struct addrinfo hints, *serv;
  struct sockaddr_storage their_addr; // connector's address information
  socklen_t sin_size;
  char s[INET6_ADDRSTRLEN];
  int rv;

  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE; // use my IP

  // Load address info
  if ((rv = getaddrinfo(NULL, _port_no.c_str(), &hints, &serv)) != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
    exit(-1);
  }

  // Create the socket
  if ((sockfd = socket(serv->ai_family, serv->ai_socktype, serv->ai_protocol)) == -1) {
    perror("server: socket");
    exit(-1);
  }

  // bind the address
  if (bind(sockfd, serv->ai_addr, serv->ai_addrlen) == -1) {
    close(sockfd);
    perror("server: bind");
    exit(-1);
  }
  freeaddrinfo(serv); // all done with this structure

  // Start listen
  if (listen(sockfd, backlog_buffer) == -1) {
    perror("listen");
    exit(-1);
  }

  printf("server: waiting for connections...\n");

  // loop to run the connection handler
  while (true) {
    sin_size = sizeof their_addr;
    int new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
    if (new_fd == -1) {
      perror("accept");
      continue;
    }
    // printf("server: got connection\n");
    NetworkRequestChannel* n = new NetworkRequestChannel(new_fd);
    pthread_t tid;
    if (pthread_create(&tid, 0, ch, n) < 0) {
      printf("server: pthread_create error");
      exit(-1);
    }
  }

}

// Slave socket
NetworkRequestChannel::NetworkRequestChannel(int sfd) {
  sockfd = sfd;
}

NetworkRequestChannel::~NetworkRequestChannel() {
  close(sockfd);
}

std::string NetworkRequestChannel::send_request(std::string _request) {
  cwrite(_request);
  std::string result = cread();
  return result;
}

std::string NetworkRequestChannel::cread() {
  char buf[1024];
  recv(sockfd, buf, 1024, 0);
  std::string answer(buf);
  // printf("received answer: %s\n", buf);
  return answer;
}
int NetworkRequestChannel::cwrite(std::string _msg) {
  const char* buf = _msg.c_str();
  send(sockfd, buf, _msg.size() + 1, 0);
}
