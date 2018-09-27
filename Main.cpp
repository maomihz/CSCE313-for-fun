#include <iostream>
#include <fstream>

#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "parser.h"

using namespace std;

struct CommandEnv {
    char last_dir[1024] = ".";
    vector<pid_t> processes;
};

int test(string t);
string runcmd(Command cmd, CommandEnv& env);

int main(int argc, char ** argv) {
    bool testflag = false;
    bool helpflag = false;
    char c;

    // Parse arguments. When specifying -t it runs test function and exit.
    while ((c = getopt(argc, argv, "th")) != -1) {
        switch (c) {
        case 't':
            testflag = true;
            break;
        case 'h':
            helpflag = true;
            break;
        case '?':
            return 1;
        default:
            abort();
        }
    }

    if (helpflag) {
        cout << "hello, world" << endl;
        cout << "Use -t to test the parser." << endl;
        return 0;
    }


    // BEGIN shell
    string t;
    CommandEnv env;
    string prompt = "\033[0;36m$ \033[0m";
    while (cout << prompt && getline(cin, t)) {
        if (t.empty()) continue;

        try {
            if (testflag) {
                test(t);
            } else {
                runcmd(parsecmd(t), env);
            }
        } catch (ParseException& e) {
            cout << "Parser error: "<< e.what() << endl;
        }
    }
}

string runcmd(Command cmd, CommandEnv& env) {
    int size = cmd.arglist.size();

    // Setup pipes. Need total of n - 1 pipes for n commands.
    // (n - 1) * 2 file descriptors
    int fds[size * 2 - 1];

    for (int i = 0; i < size - 1; i++) {
        pipe(fds + i * 2);
    }

    for (int i = 0; i < size; i++) {
        vector<string>& line = cmd.arglist.at(i);


        // if special command, execute directly.
        string program = line.at(0);

        if (program == "cd") {
            string dir;
            if (line.size() <= 1) {
                char* home = getenv("HOME");
                if (home == NULL) {
                    cerr << "Cannot change to home directory." << endl;
                    continue;
                }
                dir = home;
            } else {
                dir = line.at(1);
                if (dir == "-") {
                    dir = env.last_dir;
                }
            }

            getcwd(env.last_dir, 1024);
            int ret = chdir(dir.c_str());
            if (ret < 0) {
                if (errno == ENOENT) {
                    cerr << "Directory " << dir << " does not exist!" << endl;
                } else {
                    cerr << "Error changing directory!" << endl;
                }
            }
            continue;
        }
        if (program == "exit") {
            exit(0);
        }

        // Convert to a c-str
        // TODO memory leak?
        char** arglist = new char*[line.size() + 1];
        arglist[line.size()] = NULL;

        for (int i = 0; i < line.size(); i++) {
            arglist[i] = new char[line.at(i).size() + 1];
            strcpy(arglist[i], line.at(i).c_str());
        }


        // Run the actual command!
        int fd = -1;
        if (!fork()) {
            // Redirect stdout, unless it's the last command
            if (i < size - 1) {
                dup2(fds[i * 2 + 1], STDOUT_FILENO);
                close(fds[i * 2]);
                close(fds[i * 2 + 1]);
            } else if (!cmd.output_redir.empty()) {
                // TODO: not closing the file after exec?
                fd = open(cmd.output_redir.c_str(),
                    O_CREAT|O_WRONLY|O_TRUNC|O_CLOEXEC, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
                dup2(fd, STDOUT_FILENO);
            }
            // Redirect stdin, unless it's the first command
            if (i > 0) {
                dup2(fds[i * 2 - 2], STDIN_FILENO);
                close(fds[i * 2 - 1]);
                close(fds[i * 2 - 2]);
            } else if (!cmd.input_redir.empty()) {
                fd = open(cmd.input_redir.c_str(), O_RDONLY|O_CLOEXEC, 0);
                dup2(fd, STDIN_FILENO);
            }

            int ret = execvp(arglist[0], arglist);
            cerr << "Exec failed." << endl;
            exit(1);
        }
    }

    for (int i = 0; i < size; i++) {
        // On the parent side, close all pipes
        if (i < size - 1) {
            close(fds[i * 2]);
            close(fds[i * 2 + 1]);
        }

        if (!cmd.background) {
            wait(NULL);
        }
    }

    return "";
}

int test(string t) {
    cout << "Parsecmd: @" << t << "@" << endl;
    cout << parsecmd(t) << endl;
    return 0;
}