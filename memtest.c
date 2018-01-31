#include "ackerman.h"

int main(int argc, char ** argv) {

  int basic block size, memory length;

  init_allocator(basic block size, memory length)

  ackerman_main(); // this is the full-fledged test. 
  // The result of this function can be found from the ackerman wiki page or https://www.wolframalpha.com/. If you are not getting correct results, that means that your allocator is not working correctly. In addition, the results should be repetable - running ackerman (3, 5) should always give you the same correct result. If it does not, there must be some memory leakage in the allocator that needs fixing
  
  // please make sure to run small test cases first before unleashing ackerman. One example would be running the following: "print_allocator (); x = my_malloc (1); my_free(x); print_allocator();" the first and the last print should be identical.
  
  release_allocator()
}
