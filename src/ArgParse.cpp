#include "ArgParse.h"
#include <string>
#include <vector>
using namespace std;

ArgParse::ArgParse() {
    fail_flag = false;
    error_message = "";
}

Argument* ArgParse::createArgument(const string &flag_name, int num_args, const string &help_message) {
    Argument *new_argument = new Argument;
    new_argument->flag_name = flag_name;
    new_argument->num_args = num_args;
    new_argument->help_message = help_message;
    // new_argument->tokens gets initialized as an empty vector
    arguments.push_back(new_argument);
    return new_argument;
}

void ArgParse::parse(const int &argc, char ** const &argv) {
    vector<string> tokens;
    vectorizeTokens(tokens, argc, argv);
    for(vector<Argument*>::iterator i = arguments.begin(); i != arguments.end(); i++) {
        if(!tryParsing(*i, tokens, argc, argv)) {
            return;
        }
    }
}

void ArgParse::vectorizeTokens(vector<string> &tokens, const int &argc, char ** const &argv) {
    for(int i = 1; i < argc; i++) { // skip argv[0]
        tokens.push_back(argv[i]);
    }
}

bool ArgParse::tryParsing(Argument *argument, const vector<string> &tokens, const int &argc, char ** const &argv) {
    for(vector<string>::const_iterator i = tokens.begin(); i != tokens.end(); i++) {
        if(argument->flag_name == *i) {
            i++;
            for(int offset = 0; offset < argument->num_args; offset++, i++) {
                if(i != tokens.end()) {
                    argument->tokens.push_back(*i);
                } else {
                    fail_flag = true;
                    error_message = "Error while parsing " + argument->flag_name + 
                        ": expected num_args=" + to_string(argument->num_args);
                    return false;
                }
            }
            return true;
        }
    }
    fail_flag = true;
    error_message = "Could not find argument " + argument->flag_name;
    return false;
}

string ArgParse::help_string() {
    string ret = "Usage Information:";
    string line;
    Argument *current;
    for(vector<Argument*>::iterator i = arguments.begin(); i != arguments.end(); i++) {
        current = *i;
        line = "  " + current->flag_name + "[" + to_string(current->num_args) + "]: " + current->help_message;
        ret += "\n" + line;
    }
    return ret;
}
