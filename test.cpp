#include <iostream>

#include "command.h"
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/stat.h>

using namespace std;

vector<string> cases = {
  "echo     hello    world  !|cat -f -c>inputredir.txt",
  "    echo  \"2|3\"   2    3 >helloworld.txt<input.txt",
  "echo '\"'\"''\"",
  "tar -c  -v  -f   \"-z\"  hello.tar.gz  hello world",
  "echo \"2    '<>'   3\"a  s \">hello.txt\">           hello.txt",
  "<hello.txt>hellowo'rld.txt' cat",
  "ps -a | awk '/pts\\/[0-9]/{print $1}' | tail -5",
  "ls -l /proc/sys | awk '{print $9}' | sort -r | head -5",
  "dd if=/dev/zero of=/dev/null bs=1024 count=10485760 & hello world",
  "pwd > pwd.txt &",
  "echo \"<<<<< Optional... >>>>>\"",
  "cat /proc/$(ps|grep bash|head -1|awk '{print $1}')/status"
};

int main() {
  cout << "hello, world" << endl;

  for (string s : cases) {
    cout << "*** BEGIN TEST CASE ***" << endl;
    cout << "Command: #" << s << "#" << endl;
    Command c(s);
    for (vector<string> arglist : c.cmdlist)  {
      cout << ">>>>> CMD <<<<<" << endl;
      for (string s : arglist) {
        cout << "- " << "#" << s << "#" << endl;
      }
    }
    if (c.output_redir) {
      cout << "Output: " << c.output_file << endl;
    }
    if (c.input_redir) {
      cout << "Input: " << c.input_file << endl;
    }
    if (c.background) {
      cout << "Running in background..." << endl;
    }
  }
  cout << "**** END TEST CASE ****" << endl;


  cout << endl << endl;
  cout << "**** Test Piping ****" << endl;
  int fds[6];
  pipe(fds);
  pipe(fds+2);
  pipe(fds+4);

  cout << fds[0] << endl;
  cout << fds[1] << endl;
  cout << fds[2] << endl;
  cout << fds[3] << endl;

  if (!fork()) {
    cout << "exec1" << endl;
    dup2(fds[1], 1);
    close(fds[1]);
    close(fds[0]);
    execlp("ls", "ls", "-l", NULL);
    cerr << "bad ls" << endl;
  }

  if (!fork()) {
    cout << "exec2" << endl;
    dup2(fds[3], 1);
    dup2(fds[0], 0);

    close(fds[3]);
    close(fds[2]);
    close(fds[1]);
    close(fds[0]);

    execlp("tr", "tr", "r", "R", NULL);
    cerr << "bad tr" << endl;
  }

  close(fds[1]);
  close(fds[0]);

  if (!fork()) {
    cout << "exec3" << endl;
    dup2(fds[2], 0);

    close(fds[3]);
    close(fds[2]);
    execlp("grep", "grep", "x", NULL);
    cerr << "bad cat" << endl;
  }

  // dup2(fds[0], 0);
  // dup2(1, fds[4]);

  // waitpid(pid, &status, 0);

  cout << "b " << endl;
  // ls -l | tr r R | grep x


}
