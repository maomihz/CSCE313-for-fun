#include <string>
#include <vector>
#include <sstream>

using namespace std;

class Command {
public:
  vector<vector<string>> cmdlist;

  bool input_redir;
  bool output_redir;
  bool background;

  string input_file;
  string output_file;

  Command(string cmd);
};

class parse_error : public runtime_error {
public:
  explicit parse_error(const char* message):
    runtime_error(message) {
    }
};
