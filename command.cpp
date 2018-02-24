#include "command.h"

Command::Command(string cmd)
  : cmdlist(),
  input_redir(false), output_redir(false), background(false),
  input_file(), output_file() {
    vector<string> arglist;

    stringstream ss(cmd);
    string token;
    char buffer[2048]; // 2KB buffer
    unsigned short buffer_i = 0;

    // Mark the states
    bool read_input_redir = false;
    bool read_output_redir = false;
    bool wait_double_quote = false;
    bool wait_single_quote = false;
    bool flag_expansion = false;
    bool wait_expansion = false;

    char c = 0;
    while (c >= 0) {
      // In the while loop we get character one by one
      c = ss.get();

      // If it's a quote then ignore and wait for quote
      if (!wait_double_quote && !wait_single_quote && !wait_expansion) {
        if (c == '"') {
          wait_double_quote = true;
          continue;
        } else if (c == '\'') {
          wait_single_quote = true;
          continue;
        }
      }


      // Stop reading if these special characters are found
      bool put_buffer = true;
      if (
        ((!wait_double_quote && !wait_single_quote && !wait_expansion) && (c == ' ' || c == '<' || c == '>' || c == '|' || c == '&' || c == '$'))
          || (!wait_single_quote && !wait_expansion && c == '"') || (!wait_double_quote && !wait_expansion && c == '\'')
          || (flag_expansion && c == '(') || (wait_expansion && c == ')') || c < 0
        ) {
        put_buffer = false;
        // Found a matching quote, end waiting quote and move on
        if (wait_double_quote && c == '"') {
          wait_double_quote = false;
          continue;
        }
        if (wait_single_quote && c == '\'') {
          wait_single_quote = false;
          continue;
        }
        flag_expansion = false;
        if (c == '$') {
          flag_expansion = true;
          put_buffer = true;
        } else if (c == '(') {
          wait_expansion = true;
          put_buffer = true;
        } else if (c == ')') {
          wait_expansion = false;
          put_buffer = true;
        }

        // If we have something to remember
        if (!put_buffer && buffer_i > 0) {
          string s(buffer, buffer_i);
          if (read_input_redir) {
            input_redir = true;
            read_input_redir = false;
            input_file = s;
          } else if (read_output_redir) {
            output_redir = true;
            read_output_redir = false;
            output_file = s;
          } else { // otherwise is an argument
            arglist.push_back(s);
          }
          // Reset the buffer
          buffer_i = 0;
        }

        // Start of input redirection
        if (c == '<') {
          read_input_redir = true;
        }
        // Start output redirection
        else if (c == '>') {
          read_output_redir = true;
        }
        // Pipeline or end of input
        else if (c == '|' || c == '&' || c < 0) {
          cmdlist.push_back(arglist);
          arglist.clear();
        }
        // & also signals the end
        else if (c == '&') {
          background = true;
          break;
        }
      }

      if (put_buffer) {
        // Just put the character in buffer and move on
        buffer[buffer_i] = c;
        ++buffer_i;
      }

    }

    if (wait_double_quote) {
      throw parse_error("unmatched double quote");
    }
    if (wait_single_quote) {
      throw parse_error("unmatched single quote");
    }
    if (wait_expansion) {
      throw parse_error("unmatched expansion");
    }
    if (read_input_redir) {
      throw parse_error("input redirection file not specified");
    }
    if (read_output_redir) {
      throw parse_error("output redirection file not specified");
    }
  }
