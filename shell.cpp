#include <iostream>
#include <string>
#include <vector>
#include <sstream>

#include "command.h"
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/stat.h>

using namespace std;


string PS1 = "$ ";

int main() {
  cout << "hello, world" << endl;

  string cmd;
  while (true) {
    cout << PS1;
    if (!getline(cin, cmd)) {
      break;
    }


    // Parse the command
    try {
      Command c(cmd);
      int n = c.cmdlist.size();

      int allfds [2 * (n - 1)];
      for (int i = 0; i < (n - 1); ++i) {
        pipe(allfds + i * 2);
      }


      for (int a = 0; a < n; ++a) {
        vector<string> arg = c.cmdlist.at(a);
        char* argv[arg.size() + 1];
        // Convert the command into c-style
        for (size_t i = 0; i < arg.size(); ++i) {
          const char* cstr = arg[i].c_str();
          argv[i] = new char[arg[i].size()];
          strcpy(argv[i], cstr);
        }
        argv[arg.size()] = NULL;



        // Child
        pid_t cpid = fork();
        if (!cpid) {
          if (a > 0) { // Not the first one
            dup2(allfds[2 * a - 2], 0);
            close(allfds[2 * a - 1]);
            close(allfds[2 * a - 2]);
          } else if (c.input_redir) { // IS the first one
            int fd = open(c.input_file.c_str(), O_RDONLY, 0);
            if (fd < 0) {
              throw runtime_error("Cannot open " + c.input_file + " for reading.");
            }
            dup2 (fd, 0);
          }

          if (a < n - 1) { // Not the last one
            dup2(allfds[2 * a + 1], 1);
            close(allfds[2 * a + 1]);
            close(allfds[2 * a]);
          } else if (c.output_redir) { // IS the last one
            int fd = open(c.output_file.c_str(), O_CREAT|O_WRONLY|O_TRUNC, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
            dup2 (fd, 1);
          }

          execvp(argv[0], argv);
          cerr << "bad exec" << endl;
        }

        // Parent close the pipes
        if (a > 0) {
          close(allfds[a * 2 - 2]);
          close(allfds[a * 2 - 1]);
        }

      }


    } catch (parse_error e) {
      cerr << "Parse error: " << e.what() << endl;
    } catch (runtime_error e) {
      cerr << e.what() << endl;
    }

  }
}
