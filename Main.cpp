#include <iostream>
#include <fstream>
#include <sstream>

#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <time.h>

#include <readline/readline.h>
#include <readline/history.h>

#include "parser.h"

using namespace std;

struct CommandEnv {
    char last_dir[1024] = ".";
    vector<pid_t> processes;
    bool force_exit = false;
};

string runcmd(Command cmd, CommandEnv& env);
void checkenv(CommandEnv& env);
char* getprompt();

int test(string t);


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
    CommandEnv env;
    char* t;
    char* prompt;
    while (printf("%s", prompt = getprompt()) && (t = readline("")) != nullptr) {
        // If nothing is read from the line
        delete [] prompt;
        if (strlen(t) <= 0) {
            if (!testflag) {
                checkenv(env);
            }
            continue;
        }

        add_history(t);
        string t_str(t);

        try {
            if (testflag) {
                test(t_str);
            } else {
                runcmd(parsecmd(t_str), env);
            }
        } catch (ParseException& e) {
            cout << "Parser error: "<< e.what() << endl;
        }
    }
}

string runcmd(Command cmd, CommandEnv& env) {
    checkenv(env);

    int size = cmd.arglist.size();

    // Do replacement on all commands
    for (pair<int, int> replacement : cmd.replace_parts) {
        string replacement_cmd_str = cmd.arglist.at(replacement.first).at(replacement.second);
        size_t replacement_start = replacement_cmd_str.find("$(");
        size_t replacement_end = replacement_cmd_str.find(")", replacement_start+1);


        string subcmd = replacement_cmd_str.substr(replacement_start + 2, replacement_end - replacement_start - 2);

        Command replacement_cmd = parsecmd(subcmd);
        pid_t subshell_pid;
        int subshell_fd[2];
        pipe(subshell_fd);

        if (!(subshell_pid = fork())) {
            dup2(subshell_fd[1], 1);
            runcmd(replacement_cmd, env);
            close(subshell_fd[0]);
            close(subshell_fd[1]);
            exit(0);
        }

        char output[512];
        ostringstream ss;
        close(subshell_fd[1]);

        int loc;
        while ((loc = read(subshell_fd[0], output, 511)) > 0) {
            output[loc - 1] = '\0';
            ss << output;
        }
        close(subshell_fd[0]);

        string output_s = ss.str();
        replacement_cmd_str.replace(replacement_start, replacement_end - replacement_start + 1, output_s);
        cmd.arglist.at(replacement.first).at(replacement.second) = replacement_cmd_str;
    }

    // Setup pipes. Need total of n - 1 pipes for n commands.
    // (n - 1) * 2 file descriptors
    int fds[size * 2 - 1];

    for (int i = 0; i < size - 1; i++) {
        pipe(fds + i * 2);
    }

    // Iterate and run all commands
    for (int i = 0; i < size; i++) {
        vector<string>& line = cmd.arglist.at(i);
        
        // if special command, execute directly.
        string program = line.at(0);

        if (program == "cd") {
            string dir;

            // If there is no argument for cd, use the HOME variable
            if (line.size() <= 1) {
                char* home = getenv("HOME");
                if (home == NULL) {
                    cerr << "Cannot change to home directory." << endl;
                    continue;
                }
                dir = home;
            } else {
                // If the argument is "-", change to the last directory
                dir = line.at(1);
                if (dir == "-") {
                    dir = env.last_dir;
                }
            }

            // Store the old pwd before changing
            getcwd(env.last_dir, 1024);
            int ret = chdir(dir.c_str());
            if (ret < 0) {
                perror("Error changing directory!");
            }
            continue;
        }

        // If exit then just exit the program.
        if (program == "exit") {
            env.force_exit = true;
            checkenv(env);
            exit(0);
        }

        // Prints all background processes
        if (program == "jobs") {
            if (env.processes.empty()) {
                cerr << "No background processes." << endl;
            }
            for (pid_t pid : env.processes) {
                cerr << "PID = " << pid << endl;
            }
            continue;
        }

        // Inject arguments
        if (program == "ls") {
            line.insert(line.begin() + 1, "--color=auto");
        }
        if (program == "grep") {
            line.insert(line.begin() + 1, "--color=auto");
        }


        // Convert vector to a c-str argument list
        char** arglist = new char*[line.size() + 1];
        arglist[line.size()] = NULL;

        for (size_t i = 0; i < line.size(); i++) {
            arglist[i] = new char[line.at(i).size() + 1];
            strcpy(arglist[i], line.at(i).c_str());
        }


        // Run the actual command!
        int pid = fork();
        if (!pid) {
            // Redirect stdout, unless it's the last command
            if (i < size - 1) {
                close(fds[i * 2]);
                dup2(fds[i * 2 + 1], STDOUT_FILENO);
                close(fds[i * 2 + 1]);
            } else if (!cmd.output_redir.empty()) {
                int fd = open(cmd.output_redir.c_str(),
                    O_CREAT|O_WRONLY|O_TRUNC|O_CLOEXEC, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
                dup2(fd, STDOUT_FILENO);
            }

            // Redirect stdin, unless it's the first command
            if (i > 0) {
                close(fds[i * 2 - 1]);
                dup2(fds[i * 2 - 2], STDIN_FILENO);
                close(fds[i * 2 - 2]);
            } else if (!cmd.input_redir.empty()) {
                int fd = open(cmd.input_redir.c_str(), O_RDONLY|O_CLOEXEC, 0);
                dup2(fd, STDIN_FILENO);
            }

            // Do the actual exec
            execvp(arglist[0], arglist);
            perror("Exec failed");
            exit(1);
        } else if (pid < 0) {
            perror("Fork failed");
            return "";
        }

        // On the parent side, close pipes
        // if (i < size - 1) {
        //     close(fds[i * 2]);
        //     close(fds[i * 2 + 1]);
        // }
        if (i > 0) {
            close(fds[i * 2 - 1]);
            close(fds[i * 2 - 2]);
        }

        // Wait for background processes
        int status;
        if (!cmd.background) {
            if (waitpid(pid, &status, 0) == -1) {
                perror("waitpid failed");
                continue;
            }
        } else {
            env.processes.push_back(pid);
        }


        // Free memories
        for (size_t i = 0; i < line.size() + 1; i++) {
            delete [] arglist[i];
        }
        delete [] arglist;
    }
 
    return "";
}


// Check background processes, reap them when necessary.
void checkenv(CommandEnv& env) {
    auto iter = env.processes.begin();

    while (iter != env.processes.end()) {
        int status;
        bool exit = false;
        pid_t pid = *iter;

        // Force exit will wait for the child to complete
        if (env.force_exit) {
            int ret = waitpid(pid, &status, 0);
            if (ret > 0) {
                exit = true;
            } else {
                perror("waitpid (force) failed.");
            }
        } else {
            int ret = waitpid(pid, &status, WNOHANG);
            if (ret > 0) {
                cerr << "Process " << pid << " exited " << WEXITSTATUS(status) << '.' << endl;
                exit = true;
            } else if (ret == -1) {
                perror("waitpid (normal) failed");
            }
        }

        // If child exits successfully, erase it from the array.
        if (exit) {
            iter = env.processes.erase(iter);
        } else {
            iter++;
        }
    }
}


char* getprompt() {
    char login[64];
    getlogin_r(login, sizeof(login) - 1);

    char cwd[256];
    getcwd(cwd, sizeof(cwd) - 1);

    time_t t = time(NULL);
    struct tm tm = *localtime(&t);

    char* prompt = new char[1024];

    sprintf(prompt, "\n\033[1;35m# \033[32m%s \033[0min \033[33m%s\033[0m [%02d:%02d:%02d]\n\033[0;36m$ \033[0m", login, cwd, tm.tm_hour, tm.tm_min, tm.tm_sec);
    // sprintf(prompt, "\n%s %s %d %d %d", login, cwd, tm.tm_hour, tm.tm_min, tm.tm_sec);
    // printf(prompt);
    return prompt;
}







int test(string t) {
    cout << "Parsecmd: @" << t << "@" << endl;
    cout << parsecmd(t) << endl;
    return 0;
}