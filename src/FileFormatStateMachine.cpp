#include "FileFormatStateMachine.h"
#include "string_common.h"
#include <string>
using namespace std;

/**
 * Auxiliary declarations
 */

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
    if(allWhitespace(line)) {
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

string delimLint(string s) {
    removeWhitespace(s);
    return toLowerCase(s);
    return s;
}
