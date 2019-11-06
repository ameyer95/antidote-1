#ifndef ARGPARSE_H
#define ARGPARSE_H

#include <map>
#include <string>
#include <vector>


struct Argument {
    std::string id; // E.g. "depth" (unused by the actual parsing)
    std::string flag_name; // E.g. "-f"
    int num_args; // How many tokens following flag_name should be included
    std::string help_message;
    std::vector<std::string> tokens; // This is populated by ArgParse::parse

    bool optional;
    bool included; // Indicates that an optional argument was present

    Argument(const std::string &id, const std::string &flag_name, int num_args, const std::string &help_message, bool optional);
    Argument() {}
};


class ArgParse {
private:
    struct TokenConstraint {
        std::string id;
        int token_index;
        bool (*fptr)(const std::string &value);
        std::string failure_message;
    };
    std::map<const std::string, Argument> arguments;
    std::vector<std::vector<std::string>> at_most_one_constraints;
    std::vector<std::vector<std::string>> at_least_one_constraints;
    std::vector<TokenConstraint> token_constraints;
    bool fail_flag;
    std::string error_message;

    void vectorizeTokens(std::vector<std::string> &tokens, const int &argc, char ** const &argv);
    bool tryParsing(Argument &argument, const std::vector<std::string> &tokens);
    void checkConstraints(); // called internally at the end of parse

public:
    ArgParse();
    
    void createArgument(const std::string &id, const std::string &flag_name, int num_args, const std::string &help_message, bool optional=false);
    void parse(const int &argc, char ** const &argv); // Populates the createdArguments

    void requireAtMostOne(const std::vector<std::string> &ids) { at_most_one_constraints.push_back(ids); }
    void requireAtLeastOne(const std::vector<std::string> &ids) { at_least_one_constraints.push_back(ids); }
    void requireTokenConstraint(const std::string &id, int token_index, bool (*fptr)(const std::string &value), const std::string &failure_message);

    const Argument& operator[](const std::string &id) const { return arguments.at(id); }
    Argument& operator[](const std::string &id) { return arguments[id]; }

    std::string help_string();

    bool failure() { return fail_flag; }
    std::string message() { return error_message; }
};


#endif
