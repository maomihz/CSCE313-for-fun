#include <string>
#include <vector>
#include <stdexcept>
#include "command.h"

#define PIPE '|'

using namespace std;

Command parsecmd(const string cmd);

class ParseException : exception {
private:
    string message;
public:
    explicit ParseException(const string& message);
    virtual const char* what() const throw() {
        return message.c_str();
    }
};