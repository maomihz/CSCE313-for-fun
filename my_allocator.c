/*
    File: my_allocator.h
    This file contains the implementation of the module "MY_ALLOCATOR".
*/

#include <stdio.h>
#include <stdlib.h>
#include "my_allocator.h"

#define HEADER_SIZE sizeof(void*) + 2 * sizeof(char)
#define FREE 0
#define USED 1

Addr memory;
Addr* free_list;
Addr* free_list_end;
int free_list_len;
unsigned int basic_block_size;
unsigned int length;


/*--------------------------------------------------------------------------*/
/* General Helper Functions */
/*--------------------------------------------------------------------------*/
// https://stackoverflow.com/questions/758001/log2-not-found-in-my-math-h
unsigned int mylog2(unsigned int v) {
    unsigned int ans = 0;
    while (v >>= 1) ans++;
    return ans;
}

// https://stackoverflow.com/questions/466204/rounding-up-to-next-power-of-2
// I could easily use a while loop but this seem to be more efficient.
// Only works for 32 bit unsigned int.
unsigned int next_power_of_2(unsigned int v) {
    v--;
    v |= v >> 1;
    v |= v >> 2;
    v |= v >> 4;
    v |= v >> 8;
    v |= v >> 16;
    v++;
    return v;
}

/*--------------------------------------------------------------------------*/
/* Header Manipulation */
/*--------------------------------------------------------------------------*/

void set_is_used(Addr addr, unsigned char is_used) {
    *((unsigned char*)(addr + sizeof(Addr*) + sizeof(char))) = is_used;
}

void set_next(Addr addr, Addr next) {
    *((Addr*)addr) = next;
}

void set_abstract_size(Addr addr, unsigned char abstract_size) {
    *((unsigned char*)(addr + sizeof(Addr*))) = abstract_size;
}

void set_size(Addr addr, unsigned int block_size) {
    unsigned char abstract_size = mylog2(block_size / basic_block_size);
    set_abstract_size(addr, abstract_size);
}

// This function just set all the information together
void write_header(Addr addr, Addr* next_pointer, unsigned int block_size, unsigned char is_used) {
    set_next   (addr, next_pointer);
    set_size   (addr, block_size);
    set_is_used(addr, is_used);
}
void write_header_abstract(Addr addr, Addr* next_pointer, unsigned char abstract_block_size, unsigned char is_used) {
    set_next         (addr, next_pointer);
    set_abstract_size(addr, abstract_block_size);
    set_is_used      (addr, is_used);
}

Addr* get_next(Addr addr) {
    return *((Addr*)addr);
}

unsigned int get_abstract_size(Addr addr) {
    return *((unsigned char*)(addr + sizeof(Addr*)));
}

unsigned int get_size(Addr addr) {
    unsigned int abstract_size = get_abstract_size(addr);
    return basic_block_size * (1 << abstract_size);
}

unsigned char get_is_used(Addr addr) {
    return *((unsigned char*)(addr + sizeof(Addr*) + sizeof(char)));
}

/*--------------------------------------------------------------------------*/
/* Specific Helper Functions */
/*--------------------------------------------------------------------------*/
// Find its buddy with bitwise or, relies on the header to find the size of
// the block
Addr* find_buddy(Addr addr) {
    // Start with offset 0
    long offset = addr - memory;
    // This case memory is illegal
    if (offset < 0) {
        fprintf(stderr, "[Buddy]: The memory address is certainly incorrect.\n");
        return NULL;
    }
    unsigned int total_abstract_size = get_abstract_size(addr) + mylog2(basic_block_size);
    long buddy = offset ^ (1 << total_abstract_size);
    return buddy + memory;
}

// Free list manipulation, add and remove blocks.

// When adding a block it always adds to the end. It would ensure the correct
// "next" in header is set (to null).
// DOES NOT check if the block is already in the list. Blindly add to the end.
void add_block(Addr addr) {
    unsigned int size = get_abstract_size(addr);
    if (free_list_end[size]) {
        // Append to the end of the free list
        set_next(free_list_end[size], addr);
    } else {
        // Otherwise just add to the front
        free_list[size] = addr;
    }
    free_list_end[size] = addr;
    // Set the correct header
    set_next(addr, NULL);
}


// When removing a block it relies on the header to find its size, and search
// through the free list to remove it. Returns 0 on success and others on failure.
// The function DOES NOT set the header. It only removes the block.
int remove_block(Addr addr) {
    unsigned int size = get_abstract_size(addr);
    // now try to find the block
    Addr prev_block = NULL;
    Addr block = free_list[size];
    while (block != addr) {
        // If block is null then either the list is empty or not in the list
        if (!block) {
            return 127; // Not Found
        }
        // Advance one element
        prev_block = block;
        block = get_next(block);
    }


    // By now the block must be found in the free list
    // Remove it by setting the correct header of the previous block
    Addr next = get_next(block);
    if (prev_block) {
        set_next(prev_block, next);
    } else { // If the previous block is null, then the block is the first in the list
        free_list[size] = next;
    }

    // If the removed block is the last in the block, then set the end to
    // be the previous block.
    if (!next) {
        free_list_end[size] = prev_block;
    }

    return 0;
}

// Split the block into two parts and return the second part.
// It DOES NOT remove the block but it will add the second block back to
// the free list. (not the first one!)
Addr* split(Addr block) {
    if (get_is_used(block) != FREE) {
        fprintf(stderr, "Are you trying to split a non-free list?\n");
        return NULL;
    }

    // Just write those headers
    unsigned int size = get_abstract_size(block);
    unsigned int half_size = size - 1;

    Addr block2 = block + get_size(block) / 2;
    // The splitted block will always be added to the end.
    write_header_abstract(block, block2, half_size, FREE);
    write_header_abstract(block2, NULL, half_size, FREE);
    add_block(block2);

    return block2; // Return the second block. Returning the first
                   // is unnecessary.
}

// DOES NOT add the merged block back!!!
Addr* merge(Addr block) {
    // The function intelligently finds its buddy and merge with it.
    // HOWEVER it will do nothing when it should not.
    // Hopefully you won't try to merge the largest block.
    if (get_is_used(block) != FREE) {
        fprintf(stderr, "[Merge]: Are you trying to merge a non-free block?\n");
        return NULL;
    }

    // Find its buddy
    int block_size = get_size(block);
    Addr buddy = find_buddy(block);

    // Check errors
    if (get_abstract_size(block) >= free_list_len // Already the largest
        || get_size(buddy) != block_size          // Not same size
        || get_is_used(buddy) == USED) {          // Buddy is busy
        return NULL;
    }

    // Finally, let's write the header
    // Remove its buddy from free list
    if (remove_block(buddy) != 0) {
        fprintf(stderr, "warning: remove block failed.\n");
    }
    // write to the left of the two merged blocks
    if (buddy < block) {
        block = buddy;
    }
    write_header(block, NULL, block_size * 2, FREE);
    // Return the left of the block
    return block;
}

unsigned int init_allocator(unsigned int _basic_block_size,
    unsigned int _length) {
        // round the length and basic block size to next power of 2
        _length = next_power_of_2(_length);
        _basic_block_size = next_power_of_2(_basic_block_size);

        // Prevent the size to be less than header size
        if (_basic_block_size <= (HEADER_SIZE)) {
            fprintf(stderr, "Oh no! Basic block %d is too small!\n", _basic_block_size);
            exit(127);
        }


        // Prevent the length to be less than basic block size
        if (_length < _basic_block_size) {
            fprintf(stderr, "Oh well! Total memory %d too small!\n", _length);
            exit(126);
        }

        // Assign the global variables
        basic_block_size = _basic_block_size;
        length = _length;

        // Initialize the big piece of memory
        memory = malloc(_length);

        // Initialize the free list
        free_list_len = 0;
        int units = _length / _basic_block_size;
        while (units) {
            units >>= 1;
            free_list_len += 1;
        }
        free_list = (Addr*)malloc(free_list_len * sizeof(Addr*));
        free_list_end = (Addr*)malloc(free_list_len * sizeof(Addr*));
        for (int i = 0; i < free_list_len; ++i) {
            free_list[i] = NULL;
            free_list_end[i] = NULL;
        }

        // Initialize the last big block
        write_header(memory, NULL, length, FREE);
        add_block(memory);

        return 0;
}
/* This function initializes the memory allocator and makes a portion of
   ’_length’ bytes available. The allocator uses a ’_basic_block_size’ as
   its minimal unit of allocation. The function returns the amount of
   memory made available to the allocator. If an error occurred,
   it returns 0.
*/



int release_allocator() {
    free(memory);
    free(free_list);
    free(free_list_end);
    return 0;
}
/* This function returns any allocated memory to the operating system.
   After this function is called, any allocation fails.
*/

extern Addr my_malloc(unsigned int _length) {
    // First find the size of the block it needs
    int total_length = _length + (HEADER_SIZE);
    int size_needed; // This is the abstract representation of size
    for (int i = 0;;++i) { // While True
        int block = basic_block_size * (1 << i);
        if (block >= total_length) {
            size_needed = i;
            break;
        }
    }


    // Let's find the block? (Size of the block needed)
    int split_size = size_needed; // This represents from which block size it
                                  // need to split
    // Find a larger block to split
    for (;;) {
        if (split_size >= free_list_len) {
            fprintf(stderr, "Not Enough Memory!\n");
            return NULL;
        }
        if (free_list[split_size]) {
            break;
        }
        split_size++;
    }

    // Retrieve and remove the block
    Addr block = free_list[split_size];
    remove_block(block);


    // We are now ready to split
    while (split_size > size_needed) {
        split_size --;  // The new blocks are smaller
        split(block); // Generate a new block
        // Split already add the block back so no need to add.
    }

    // By now there must be a block of that size.
    // Mark it as used
    set_is_used(block, USED);

    return block + (HEADER_SIZE);
}

extern int my_free(Addr _a) {
    // Return to the actual block address
    Addr block = _a - (HEADER_SIZE);
    set_is_used(block, FREE);

    // Let it merge!
    for (;;) { // While true
        Addr new_block = merge(block);
        if (!new_block) {
            break;
        }
        block = new_block;
    }

    // Add the block back to the free list
    add_block(block);
    return 0;
}

void print_allocator () {
    for (int i = 0; i < free_list_len; ++i) {
        int count = 0;
        Addr addr = free_list[i];
        while (addr) {
            count ++;
            addr = get_next(addr);
        }
        printf("%d: %d\n", basic_block_size * (1<<i), count);
    }
    printf("\n");
}
