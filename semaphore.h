/*
  File: semaphore.H

  Author: R. Bettati
      Department of Computer Science
      Texas A&M University
  Date  : 08/02/11

*/

#ifndef _semaphore_H_           // include file only once
#define _semaphore_H_

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include <pthread.h>

/*--------------------------------------------------------------------------*/
/* DATA STRUCTURES */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* FORWARDS */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* CLASS   s e m a p h o r e  */
/*--------------------------------------------------------------------------*/

class semaphore {
private:
  /* -- INTERNAL DATA STRUCTURES */
  pthread_mutex_t mutex;  // mutex
  pthread_cond_t cond;    // wait queue
  int counter;            // counter

public:

  /* -- CONSTRUCTOR/DESTRUCTOR */

  semaphore(int _val)
  : counter(_val) {
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cond, NULL);
  }

  ~semaphore(){
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond);
  }

  /* -- SEMAPHORE OPERATIONS */

  // P = decrement counter, retrieve resource
  void P() {
    // Lock mutex
    pthread_mutex_lock(&mutex);
    // Decrement counter
    counter--;

    // if counter below zero then wait
    if (counter < 0) {
      pthread_cond_wait(&cond, &mutex);
    }
    // Upon receiving signal, release
    pthread_mutex_unlock(&mutex);
  }

  // V = increment counter, add resource
  void V() {
    // Lock mutex
    pthread_mutex_lock(&mutex);
    counter++;

    // If counter become 0
    if (counter == 0) {
      pthread_cond_broadcast(&cond);
    }

    // And then release
    pthread_mutex_unlock(&mutex);
  }

  // Return the value of counter, debug purpose
  int val() {
    return counter;
  }
};

#endif
