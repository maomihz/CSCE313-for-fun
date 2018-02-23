#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define GREETING "hello, world\n"

int main(int argc, char* argv[]) {

    int opt;
    int special = 0;
    int say_hello = 0;
    int echo_chars = 0;
    char *hello = NULL;
    while ((opt = getopt(argc, argv, "seo:")) != -1) {
        switch(opt) {
            case 's':
                special = 1;
                break;
            case 'e':
                echo_chars = 1;
                break;
            case 'o':
                say_hello = 1;
                hello = optarg;
                break;
            case '?':
                return 1;
        }
    }

    /* Echo Characters */
    if (echo_chars) {
        char c;
        while ((c = getchar()) != EOF) {
            putchar(c);
        }
        return 0;
    }


    /* Simply print "hello, world" */
    if (!special) {
        if (!say_hello) {
            hello = GREETING;
        }
        printf("%s", hello);
        return 0;
    }


    /* Special Code */
    printf("*** Special Code Activated ***");
    int x = 0;
    for (int i = 0; i < 10; ++i) {
        x += 15 + i;
        printf("x = %d\n", x);
    }


    /* Test Malloc */
    int s = 4 * sizeof(int);
    void *arr = malloc(s);
    int *arrint = (int*)arr;
    for (int i = 0; i < 4; ++i) {
        *(arrint + i) = i;
    }
    printf("int  : %lu\n", sizeof(int));
    printf("int* : %lu\n", sizeof(int*));
    printf("void*: %lu\n", sizeof(void*));



    /* Print some results */
    printf("hello, world. x = %d\n", x);

    char *cs = (char *)malloc(1024);
    strcpy(cs, "hello, world. nrcopy\n");

    for (int i = 0; cs[i] != '\0'; ++i) {
        putchar(cs[i]);
    }

    return 0;
}
