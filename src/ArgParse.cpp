#include "ArgParse.h"
#include <algorithm>
#include <map>
#include <string>
#include <utility>
#include <vector>
using namespace std;

/**
 * The Argument struct constructor
 */

Argument::Argument(const std::string &id, const std::string &flag_name, int num_args, const std::string &help_message, bool optional) {
    this->id = id;
    this->flag_name = flag_name;
    this->num_args = num_args;
    this->help_message = help_message;
    // tokens gets initialized as an empty vector
    this->optional = optional;
    this->included = false;
}

/**
 * The actual ArgParse members
 */

ArgParse::ArgParse() {
    fail_flag = false;
    error_message = "";
}

void ArgParse::createArgument(const string &id, const string &flag_name, int num_args, const string &help_message, bool optional) {
    Argument new_argument(id, flag_name, num_args, help_message, optional);
    arguments.insert(make_pair(id, new_argument));
}

void ArgParse::parse(const int &argc, char ** const &argv) {
    vector<string> tokens;
    vectorizeTokens(tokens, argc, argv);
    for(auto i = arguments.begin(); i != arguments.end(); i++) {
        if(!tryParsing(i->second, tokens)) {
            return;
        }
    }
    checkConstraints();
}

void ArgParse::checkConstraints() {
    for(auto i = at_least_one_constraints.cbegin(); i != at_least_one_constraints.cend(); i++) {
        if(none_of(i->cbegin(), i->cend(), [this](const std::string &id){ return this->arguments[id].included; })) {
            fail_flag = true;
            error_message = "Must specify at least one of";
            for(auto j = i->cbegin(); j != i->cend(); j++) {
                error_message += " " + arguments[*j].flag_name;
            }
            return;
        }
    }
    for(auto i = at_most_one_constraints.cbegin(); i != at_most_one_constraints.cend(); i++) {
        int num_present = 0;
        for(auto j = i->cbegin(); j != i->cend(); j++) {
            if(arguments[*j].included) {
                num_present++;
            }
        }
        if(num_present > 1) {
            fail_flag = true;
            error_message = "Must specify at most one of";
            for(auto j = i->cbegin(); j != i->cend(); j++) {
                error_message += " " + arguments[*j].flag_name;
            }
            return;
        }
    }
    for(auto i = token_constraints.cbegin(); i != token_constraints.cend(); i++) {
        if(arguments[i->id].included) {
            if(!(i->fptr)(arguments[i->id].tokens[i->token_index])) {
                fail_flag = true;
                error_message = i->failure_message;
                return;
            }
        }
    }
    for(auto i = token_set_constraints.cbegin(); i != token_set_constraints.cend(); i++) {
        if(arguments[i->id].included) {
            std::string token = arguments[i->id].tokens[i->token_index];
            if(none_of(i->values.cbegin(), i->values.cend(), [&token](const std::string &value){ return value == token; })) {
                fail_flag = true;
                error_message = "Token index " + to_string(i->token_index) + " of " + arguments[i->id].flag_name
                    + " must be one of";
                for(auto j = i->values.cbegin(); j != i->values.cend(); j++) {
                    error_message += " " + *j;
                }
                return;
            }
        }
    }
}

void ArgParse::requireTokenConstraint(const std::string &id, int token_index, bool (*fptr)(const std::string &value), const std::string &failure_message) {
    TokenConstraint temp = { id, token_index, fptr, failure_message };
    token_constraints.push_back(temp);
}

void ArgParse::requireTokenInSet(const std::string &id, int token_index, const std::set<std::string> &values) {
    TokenInSetConstraint temp = { id, token_index, values };
    token_set_constraints.push_back(temp);
}

void ArgParse::vectorizeTokens(vector<string> &tokens, const int &argc, char ** const &argv) {
    for(int i = 1; i < argc; i++) { // skip argv[0]
        tokens.push_back(argv[i]);
    }
}

// Returns false and sets a failure flag and error message in the event of an error
bool ArgParse::tryParsing(Argument &argument, const vector<string> &tokens) {
    for(auto i = tokens.begin(); i != tokens.end(); i++) {
        if(argument.flag_name == *i) {
            // We found the argument in the tokens
            argument.included = true;
            // Now read the values passed with it
            i++;
            for(int offset = 0; offset < argument.num_args; offset++, i++) {
                if(i != tokens.end()) {
                    argument.tokens.push_back(*i);
                } else {
                    fail_flag = true;
                    error_message = "Error while parsing " + argument.flag_name +
                        ": expected num_args=" + to_string(argument.num_args);
                    return false;
                }
            }
            return true;
        }
    }
    // If we reach here, the argument was not among the passed tokens
    if(argument.optional) {
        argument.included = false;
        return true;
    }
    fail_flag = true;
    error_message = "Could not find (non-optional) argument " + argument.flag_name;
    return false;
}

string ArgParse::help_string() {
    string ret = "Usage Information:";
    for(auto i = arguments.begin(); i != arguments.end(); i++) {
        Argument *current = &(i->second);
        string line = "  " + current->flag_name + "[" + to_string(current->num_args) + "]: " + current->help_message
            + (current->optional ? " (optional)" : "");
        ret += "\n" + line;
    }
    return ret;
}
