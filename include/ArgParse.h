#ifndef ARGPARSE_H
#define ARGPARSE_H

#include <string>
#include <vector>


struct Argument {
    std::string flag_name; // E.g. "-f"
    int num_args; // How many tokens following flag_name should be included
    std::string help_message;
    std::vector<std::string> tokens; // This is populated by ArgParse::parse

    bool optional;
    bool included; // Indicates that an optional argument was present
};


class ArgParse {
private:
    std::vector<Argument*> arguments;
    bool fail_flag;
    std::string error_message;

    void vectorizeTokens(std::vector<std::string> &tokens, const int &argc, char ** const &argv);
    bool tryParsing(Argument *argument, const std::vector<std::string> &tokens, const int &argc, char ** const &argv);

public:
    ArgParse();
    
    // Keep track of the references created by this to read the values,
    // and the caller is responsible for deallocation
    Argument* createArgument(const std::string &flag_name, int num_args, const std::string &help_message, bool optional=false);
    void parse(const int &argc, char ** const &argv); // Populates those references

    std::string help_string();

    bool failure() { return fail_flag; }
    std::string message() { return error_message; }
};


#endif
