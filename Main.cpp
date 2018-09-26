#include <iostream>
#include <unistd.h>
#include <fstream>

#include "parser.h"

using namespace std;

int test();

int main(int argc, char ** argv) {
    bool testflag;
    char c;

    // Parse arguments. When specifying -t it runs test function and exit.
    while ((c = getopt(argc, argv, "t")) != -1) {
        switch (c) {
        case 't':
            testflag = 1;
            break;
        case '?':
            return 1;
        default:
            abort();
        }
    }

    if (testflag) {
       return test();
    }

    cout << "hello, world" << endl;
    cout << "Use -t to test the parser." << endl;
}

int test() {
    cout << "*** BEGIN TEST ***" << endl;


    string t;
    while (getline(cin, t)) {
        if (t.empty()) continue;

        cout << "Parsecmd: @" << t << "@" << endl;

        try {
            cout << parsecmd(t) << endl;
        } catch (ParseException& e) {
            cout << "Parser error: "<< e.what() << endl;
        }
    }

    return 0;
}