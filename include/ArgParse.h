#ifndef ARGPARSE_H
#define ARGPARSE_H

#include <string>
#include <vector>
using namespace std;

struct Argument {
    string flag_name; // E.g. "-f"
    int num_args; // How many tokens following flag_name should be included
    string help_message;
    vector<string> tokens; // This is populated by ArgParse::parse

    bool optional;
    bool included; // Indicates that an optional argument was present
};


class ArgParse {
private:
    vector<Argument*> arguments;
    bool fail_flag;
    string error_message;

    void vectorizeTokens(vector<string> &tokens, const int &argc, char ** const &argv);
    bool tryParsing(Argument *argument, const vector<string> &tokens, const int &argc, char ** const &argv);

public:
    ArgParse();
    
    // Keep track of the references created by this to read the values,
    // and the caller is responsible for deallocation
    Argument* createArgument(const string &flag_name, int num_args, const string &help_message, bool optional=false);
    void parse(const int &argc, char ** const &argv); // Populates those references

    string help_string();

    bool failure() { return fail_flag; }
    string message() { return error_message; }
};


#endif
