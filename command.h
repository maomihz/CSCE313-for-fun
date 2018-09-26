#include <vector>
#include <string>

using namespace std;

class Command {
public:
    vector<vector<string> > arglist;
    string input_redir;
    string output_redir;
    bool background = false;
};

ostream& operator<<(ostream& os, const Command& cmd);
