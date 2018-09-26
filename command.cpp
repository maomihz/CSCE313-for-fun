#include "command.h"
#include <iostream>

// Print the command arg list
ostream& operator<<(ostream& os, const Command& cmd) {
    for (const vector<string>& line : cmd.arglist) {
        os << "{";
        for (int i = 0; i < line.size(); i++) {
            os << line.at(i);
            if (i < line.size() - 1) {
                os << ", ";
            }
        }
        os << "}";
        os << endl;
    }
    cout << "Input: " << cmd.input_redir << endl;
    cout << "Output: " << cmd.output_redir << endl;
    cout << "Background: " << (cmd.background ? "Yes" : "No") << endl;
    return os;
}