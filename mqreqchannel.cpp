#include "mqreqchannel.h"


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
    perror("MQ Key Error");
  }
  // If server side, then write server read client
  if (side == SERVER_SIDE) {
    wkey = ftok("server.txt", num_key);
    rkey = ftok("client.txt", num_key);
  // Otherwise, read server and write client
  } else {
    rkey = ftok("server.txt", num_key);
    wkey = ftok("client.txt", num_key);
  }

  // Create the mssage queues
  rmsqid = msgget(rkey, 0644 | IPC_CREAT);
  wmsqid = msgget(wkey, 0644 | IPC_CREAT);
  if (rmsqid < 0 || wmsqid < 0) {
    perror ("Message Queue could not be created");
  }

  // printf ("RMessage Queued ID: %ld\n", rmsqid);
  // printf ("WMessage Queued ID: %ld\n", wmsqid);


}


RequestChannel::~RequestChannel() {
  // Delete the message queues
  msgctl(rmsqid, IPC_RMID, NULL);
  msgctl(wmsqid, IPC_RMID, NULL);
}

std::string RequestChannel::send_request(std::string request) {
  cwrite(request);
  return cread();
}

// Return the # characters
int RequestChannel::cwrite(std::string msg) {
  // Prepare buffer
  struct mymsgbuf buf;
  buf.mtype = 1;
  strcpy(buf.mtext, msg.c_str());
  // std::cout << "Ready to send message: " << msg << std::endl;

  int len = strlen(buf.mtext);
  if (msgsnd(wmsqid, &buf, len + 1, 0) == -1) {
    perror("msgsnd");
  }

  // std::cout << "Message Sent: " << msg << std::endl;

  return len;
}

std::string RequestChannel::cread() {
  // Prepare buffer
  struct mymsgbuf buf;
  if (msgrcv(rmsqid, &buf, sizeof buf.mtext, 0, 0) <= 0) {
    perror("msgrcv");
    exit(1);
  }
  // std::cout << "Message Get: " << buf.mtext << std::endl;
  return std::string(buf.mtext);

}

std::string RequestChannel::name() {
  return std::to_string(rmsqid);
}
