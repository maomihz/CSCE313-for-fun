#include "shmreqchannel.h"


RequestChannel::RequestChannel(const std::string name, const Side side)
{
  int num_key = -1;
  if (name == "control") {
    num_key = 12345;
  }

  if (name.compare(0, 4, "data") == 0) {
    std::stringstream ss(name);
    for (int i = 0; i < 4; ++i) {
      ss.get();
    }
    ss >> num_key;
  }

  if (num_key == -1) {
    perror("SHM Key Error");
    exit(1);
  }
  // If server side, then write server read client
  if (side == SERVER_SIDE) {
    wkey = ftok("server.txt", num_key);
    wkey2 = ftok("server2.txt", num_key);
    rkey = ftok("client.txt", num_key);
    rkey2 = ftok("client2.txt", num_key);
  // Otherwise, read server and write client
  } else {
    rkey = ftok("server.txt", num_key);
    rkey2 = ftok("server2.txt", num_key);
    wkey = ftok("client.txt", num_key);
    wkey2 = ftok("client2.txt", num_key);
  }

  // Create shared [memories]
  rshmid = shmget(rkey, SHM_SIZE, 0644 | IPC_CREAT);
  wshmid = shmget(wkey, SHM_SIZE, 0644 | IPC_CREAT);

  if (rshmid < 0 || wshmid < 0) {
    perror ("Shared Memory could not be created");
  }

  // printf ("RSHM ID: %ld\n", rshmid);
  // printf ("WSHM ID: %ld\n", wshmid);

  rsem_full = new KernelSemaphore(0, rkey);
  wsem_full = new KernelSemaphore(0, wkey);
  rsem_empty = new KernelSemaphore(1, rkey2);
  wsem_empty = new KernelSemaphore(1, wkey2);


}


RequestChannel::~RequestChannel() {
  // Delete the message queues
  shmctl(rshmid, IPC_RMID, 0);
  shmctl(wshmid, IPC_RMID, 0);
  delete rsem_full;
  delete wsem_full;
  delete rsem_empty;
  delete wsem_empty;
}

std::string RequestChannel::send_request(std::string request) {
  cwrite(request);
  return cread();
}

// Return the # characters
int RequestChannel::cwrite(std::string msg) {
  // Prepare buffer
  char* data = (char*)shmat(wshmid, 0, 0);
  // if (data == -1) {
  //   perror("cwrite shmat");
  //   exit(1);
  // }

  // std::cout << "Prepare to send message: " << msg << std::endl;

  wsem_empty->P();
  strncpy(data, msg.c_str(), SHM_SIZE);
  wsem_full->V();

  // std::cout << "Ready to send message: " << msg << std::endl;

  if (shmdt(data) == -1) {
    perror("shmdt WRITE");
    exit(1);
  }
  // std::cout << "Message Sent: " << msg << std::endl;

  return msg.size();
}

std::string RequestChannel::cread() {
  char* data = (char*)shmat(rshmid, 0, 0);
  // if (data == -1) {
  //   perror("cread shmat");
  //   exit(1);
  // }

  rsem_full->P();
  std::string result(data);
  rsem_empty->V();

  if (shmdt(data) == -1) {
    perror("shmdt READ");
    exit(1);
  }

  // std::cout << "Message Get: " << data << std::endl;
  return result;

}

std::string RequestChannel::name() {
  return std::to_string(rshmid);
}
