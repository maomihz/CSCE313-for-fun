#include <iostream>
#include <string>
#include <vector>
#include <sstream>

#include "command.h"
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/stat.h>

using namespace std;


string PS1 = "\x1b[36m$\x1b[0m ";
vector<pid_t> bg_process;
char cd_last[1024];


void checkbg() {
    int status;
    auto i = bg_process.begin();
    while (i != bg_process.end()) {
      if (waitpid(*i,&status,WNOHANG)) {
        bg_process.erase(i);
        cerr << "[shell] PID=" << *i << " exit " << status << endl;
      } else {
        ++i;
      }
    }
}

string exec_out(char* const* cmd) {
  int fds[2];
  pipe(fds);

  pid_t p;
  int stat;
  if (!(p = fork())) {
    dup2 (fds[1], 1);
    execvp(cmd[0], cmd);
  }

  char buf[256];
  read(fds[0], buf, 256);
  string result = buf;
  waitpid(p, &stat, 0);
  return result;
}

int main(int argc, char** argv) {
  atexit(checkbg);

  // Parse arguments
  char c;
  while ((c = getopt(argc, argv, "tp:")) != -1) {
    switch (c) {
      case 't':
        PS1="";
        break;
      case 'p':
        PS1=optarg;
        break;
    }
  }

  string cmd;
  while (true) {
    // Gather infomations

    // Printing the prompt
    // char* arl1[] = {(char*)"date", (char*)"+%F", NULL};
    // string prompt_date = exec_out(arl1);
    //
    // char* arl2[] = {(char*)"pwd", NULL};
    // string prompt_pwd = exec_out(arl2);
    //
    // char* arl3[] = {(char*)"whoami", NULL};
    // string prompt_whoami = exec_out(arl3);
    //
    // char* arl4[] = {(char*)"date", (char*)"+%T", NULL};
    // string prompt_time = exec_out(arl4);

    string prompt = PS1;
    cout << prompt;
    // cout << prompt_date << endl;
    // cout << prompt_pwd << endl;
    // cout << prompt_whoami << endl;
    // cout << prompt_time << endl;


    if (!getline(cin, cmd)) {
      break;
    }


    try {
      Command c(cmd);
      int n = c.cmdlist.size();

      if (n <= 0 || (n == 1 && c.cmdlist.at(0).size() <= 0)) {
        checkbg();
        continue;
      }

      // Initialize pipes
      int fds [2 * (n - 1)];
      for (int i = 0; i < (n - 1); ++i) {
        pipe(fds + i * 2);
      }

      // A vector to store processes to wait
      vector<pid_t> processes;

      // Parse the command, convert to c-style
      for (int a = 0; a < n; ++a) {
        vector<string> arg = c.cmdlist.at(a);
        if (arg.size() <= 0) break;
        // Builtins
        if (arg.at(0) == "exit") {
          exit(0);
        }
        if (arg.at(0) == "cd") {
          if (arg.size() < 2) {
            throw runtime_error("cd: Please specify a directory.");
          }
          string dir = "";
          if (arg.at(1) == "-") {
            dir = cd_last;
          } else {
            dir = arg.at(1);
          }

          if (dir.size() > 0) {
            getcwd(cd_last, sizeof(cd_last));
            if (chdir(dir.c_str()) < 0) {
              throw runtime_error("cd: Error changing directory");
            };
          }
          checkbg();
          continue;
        }
        if (arg.at(0) == "jobs") {
          checkbg();
          auto i = bg_process.begin();
          while (i != bg_process.end()) {
            cout << "[job]: PID="<< *i << endl;
            ++i;
          }
          continue;
        }

        char* myargv[arg.size() + 1];
        for (size_t i = 0; i < arg.size(); ++i) {
          const char* cstr = arg[i].c_str();
          myargv[i] = new char[arg[i].size()];
          strcpy(myargv[i], cstr);
        }
        myargv[arg.size()] = NULL;


        // Child
        pid_t cpid = fork();
        if (!cpid) {
          if (a > 0) { // Not the first one
            dup2(fds[2 * a - 2], 0);
            close(fds[2 * a - 1]);
            close(fds[2 * a - 2]);
          } else if (c.input_redir) { // IS the first one
            int fd = open(c.input_file.c_str(), O_RDONLY, 0);
            if (fd < 0) {
              throw runtime_error("Cannot open " + c.input_file + " for reading.");
            }
            dup2 (fd, 0);
          }

          if (a < n - 1) { // Not the last one
            dup2(fds[2 * a + 1], 1);
            close(fds[2 * a + 1]);
            close(fds[2 * a]);
          } else if (c.output_redir) { // IS the last one
            int fd = open(c.output_file.c_str(), O_CREAT|O_WRONLY|O_TRUNC, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
            dup2 (fd, 1);
          }

          execvp(myargv[0], myargv);
          cerr << "Exec failed: " << myargv[0] << endl;
          exit(1);
        }

        // Parent close the pipes
        processes.push_back(cpid);
        if (a > 0) {
          close(fds[a * 2 - 2]);
          close(fds[a * 2 - 1]);
        }

        if (!c.background) {
          int status;
          for (pid_t p : processes) {
            waitpid(p, &status, 0);
          }
        } else {
          for (pid_t p : processes) {
            bg_process.push_back(p);
          }
        }

        // Free the "new"
        for (size_t i = 0; i < arg.size(); ++i) {
          delete [] myargv[i];
        }

        checkbg();

      }


    } catch (parse_error e) {
      cerr << "Parse error: " << e.what() << endl;
    } catch (runtime_error e) {
      cerr << e.what() << endl;
    }

  }
}
