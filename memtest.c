#include "ackerman.h"
#include "my_allocator.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

void mytest();

int main(int argc, char ** argv) {
    // Parse command arguments
    char c;
    char *bs = NULL, *ml = NULL;
    int bsflag = 0, mlflag = 0, tflag = 0;
    unsigned int basic_block_size = 128;
    unsigned int memory_length = 524288; // Default 512 KB
    // 1 GB = 1073741824

    while ((c = getopt(argc, argv, "b:s:t")) != -1) {
        switch (c) {
            case 'b':
                bs = optarg;
                bsflag = 1;
                break;
            case 's':
                ml = optarg;
                mlflag = 1;
                break;
            case 't':
                tflag = 1;
                break;
            default:
                return 1;
        }
    }

    if (bsflag) {
        basic_block_size = atoi(bs);
    }
    if (mlflag)  {
        memory_length = atoi(ml);
    }

    printf("Basic Block Size: %u, Memory Length: %u\n", basic_block_size, memory_length);


    init_allocator(basic_block_size, memory_length);
    print_allocator();

    if (tflag) {
        mytest();
    } else {
        ackerman_main(); // this is the full-fledged test.
        print_allocator();
    }

    // The result of this function can be found from the ackerman wiki page or https://www.wolframalpha.com/. If you are not getting correct results, that means that your allocator is not working correctly. In addition, the results should be repetable - running ackerman (3, 5) should always give you the same correct result. If it does not, there must be some memory leakage in the allocator that needs fixing

    // please make sure to run small test cases first before unleashing ackerman. One example would be running the following: "print_allocator (); x = my_malloc (1); my_free(x); print_allocator();" the first and the last print should be identical.

    release_allocator();
}





void mytest() {
    Addr a = my_malloc(399);
    print_allocator();
    Addr b = my_malloc(256);
    print_allocator();
    Addr c = my_malloc(2048);
    print_allocator();
    Addr d = my_malloc(66);
    print_allocator();
    //
    printf("let's free them\n");
    my_free(a);
    print_allocator();
    my_free(b);
    print_allocator();
    my_free(c);
    print_allocator();
    my_free(d);
    print_allocator();
}
