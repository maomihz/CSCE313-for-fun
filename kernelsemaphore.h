#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>

class KernelSemaphore {
private:
  int semid;
public:
  KernelSemaphore (short value, key_t key) {
    semid = semget(key, 1, IPC_CREAT | IPC_EXCL | 0666);
    if (semid < 0){
      if (errno == EEXIST) {
        semid = semget(key, 1, 0666);
      } else {
        printf ("Cannot create semaphore, %d\n", errno);
        exit (-1);
      }
      if (semid < 0) {
        printf ("Cannot create semaphore Again, %d\n", errno);
        exit (-1);
      }
    } else {
      struct sembuf sb = {0, value, 0};
      if (semop(semid, &sb, 1) == -1) {
        exit(-1); /* error, check errno */
      }
    }
    // printf ("SEM created: %d (key=%d)\n", semid, key);

  }

  ~KernelSemaphore() {
    // Ignore repeat remove!
    if (semctl(semid, 0, IPC_RMID, 0) == -1 && errno != EINVAL) {
      perror("semctl destroy");
      exit(1);
    }
  }

  // Retrieve, decrement
  void P() {
    struct sembuf sb = {0, -1, 0};
    if (semop(semid, &sb, 1) == -1) {
      perror("semop P");
      exit(1);
    }
  }

  // Fill, increment
  void V() {
    struct sembuf sb = {0, 1, 0};
    if (semop(semid, &sb, 1) == -1) {
      perror("semop V");
      exit(1);
    }
  }

  int id() {
    return semid;
  }
};
