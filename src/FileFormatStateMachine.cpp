#include "FileFormatStateMachine.h"
#include <algorithm> // all_of, remove_if, transform, ...
#include <ctype.h> // for isspace
#include <locale> // for tolower
#include <string>
using namespace std;

/**
 * Auxiliary declarations
 */

bool allWhiteSpace(const string &s);
string delimLint(string s);


/**
 * FileFormatStateMachine members
 */

FileFormatStateMachine::FileFormatStateMachine() {
    reset();
}

FileFormatStateMachine::LineType FileFormatStateMachine::processLine(const string &line) {
    if(error_flag) {
        return LineType::SKIP;
    }
    if(allWhiteSpace(line)) {
        return LineType::SKIP;
    }

    switch(current_state) {
        case State::READ_FEATURE_LINE:
            current_state = State::READ_CLASS_LABELS_LINE;
            return LineType::FEATURE_LINE;

        case State::READ_CLASS_LABELS_LINE:
            current_state = State::READ_TRAINING_DELIM;
            return LineType::CLASS_LABELS_LINE;

        case State::READ_TRAINING_DELIM:
            if(delimLint(line) == "@training") {
                current_state = State::READ_TRAINING_LINES;
            } else {
                error_flag = true;
            }
            return LineType::SKIP;
            
        case State::READ_TRAINING_LINES:
            if(delimLint(line) == "@test") {
                current_state = State::READ_TEST_LINES;
                return LineType::SKIP;
            }
            return LineType::TRAINING_LINE;

        case State::READ_TEST_LINES:
            return LineType::TEST_LINE;
    }
}


/**
 * Auxiliary definitions
 */

inline bool allWhiteSpace(const string &s) {
    return std::all_of(s.begin(), s.end(), ::isspace);
}

string delimLint(string s) {
    // remove_if can't modify string length, isspace has overloads---see https://stackoverflow.com/a/83538
    s.erase(remove_if(s.begin(), s.end(), ::isspace), s.end()); // remove whitespace
    transform(s.begin(), s.end(), s.begin(), ::tolower); // tolowercase
    return s;
}
