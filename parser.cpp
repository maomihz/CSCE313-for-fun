#include "parser.h"

#include <sstream>
#include <iostream>

ParseException::ParseException(const string& message) : message(message) {}

Command parsecmd(const string cmdstr) {
    // Setup structures
    istringstream ss(cmdstr);
    vector<string> arglist;
    Command cmd;

    char c;

    bool wait_input = false;
    bool wait_output = false;
    char wait_quote = 0;
    

    ostringstream buffer;

    // DON'T touch this part of the program.
    // It works! But I have no idea why it works.
    // This is an example of very bad coding.
    while(ss) {
        // Read characters one by one
        ss.get(c);

        // Setting any of these three variables cause the
        // character to skip
        bool separator = false;
        bool end_of_command = false;
        bool skip = false;

        bool will_wait_input = false;
        bool will_wait_output = false;



        if (!ss) {
            // If it is the last character, then force it to be
            // end of command.
            if (wait_quote) {
                throw ParseException("Unterminated Quote");
            }
            separator = true;
            end_of_command = true;
        } else if (wait_quote) {
            // While waiting for a matching quote, any
            // characters are read literally, unless a
            // matching quote appears
            if (c == wait_quote) {
                wait_quote = 0;

                // For quotes it skips the quotation marks
                skip = true;
                if (c == ')') {
                    skip = false;
                }
            }
        } else if (c == '"' || c == '\'') {
            // If not waiting for a quote
            wait_quote = c;
            skip = true;
        } else if (c == '$') {
            // $ and ( must be together to form a quote
            if (ss.peek() == '(') {
                wait_quote = ')';
            }
        } else if (c == '&') {
            // Force & to be a seperator
            separator = true;
            end_of_command = true;
            cmd.background = true;
        } else if (c == '<') {
            // For input and output, "wait" for them happens after
            // the seperation. Therefore special code need to handle
            // this.
            separator = true;
            will_wait_input = true;
        } else if (c == '>') {
            separator = true;
            will_wait_output = true;
        } else {
            end_of_command = c == '|';
            separator = end_of_command || c == ' ' || c == '\t';
        }


        // If stream is empty then do no seperation, but
        // skip the character
        if (separator && buffer.str().empty()) {
            separator = false;
            skip = true;
        }

        // If a separator is found
        if (separator) {
            if (wait_input) {
                cmd.input_redir = buffer.str();
                wait_input = false;
            } else if (wait_output) {
                cmd.output_redir = buffer.str();
                wait_output = false;
            } else {
                arglist.push_back(buffer.str());
            }
            buffer.str("");
            buffer.clear();
        }

        
        // Same thing for end of command
        if (end_of_command && arglist.empty()) {
            end_of_command = false;
            skip = true;
        }

        // If end of command is found
        if (end_of_command) {
            cmd.arglist.push_back(arglist);
            arglist.clear();
        }



        if (!skip && !separator && !end_of_command) {
            buffer << c;
        }

        if (will_wait_input) {
            wait_input = true;
        } else if (will_wait_output) {
            wait_output = true;
        }

        if (wait_output && wait_input) {
            throw ParseException("Invalid redirection syntax");
        }

    }

    // More exceptions
    if (wait_input) {
        throw ParseException("Input redirection not specified.");
    }
    if (wait_output) {
        throw ParseException("Output redirection not specified.");
    }


    return cmd;
}