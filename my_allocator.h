/*
    File: my_allocator.h
*/

#ifndef _my_allocator_h_                   // include file only once
#define _my_allocator_h_

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */


/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/


/*--------------------------------------------------------------------------*/
/* DATA STRUCTURES */
/*--------------------------------------------------------------------------*/

typedef void * Addr;

/*--------------------------------------------------------------------------*/
/* FORWARDS */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* MODULE   MY_ALLOCATOR */
/*--------------------------------------------------------------------------*/

unsigned int init_allocator(unsigned int _basic_block_size,
                unsigned int _length);
/* This function initializes the memory allocator and makes a portion of
   ’_length’ bytes available. The allocator uses a ’_basic_block_size’ as
   its minimal unit of allocation. The function returns the amount of
   memory made available to the allocator. If an error occurred,
   it returns 0.
*/

int release_allocator();
/* This function returns any allocated memory to the operating system.
   After this function is called, any allocation fails.
*/

Addr my_malloc(unsigned int _length);
/* Allocate _length number of bytes of free memory and returns the
   address of the allocated portion. Returns 0 when out of memory. */

int my_free(Addr _a);
/* Frees the section of physical memory previously allocated
   using ’my_malloc’. Returns 0 if everything ok. */

void print_allocator ();
/* Mainly used for debugging purposes and running short test cases */
/* This function should print how many free blocks of each size belong to the allocator
at that point. The output format should be the following (assuming basic block size = 128 bytes):

128: 5
256: 0
512: 3
1024: 0
....
....
 which means that at point, the allocator has 5 128 byte blocks, 3 512 byte blocks and so on.*/

#endif
