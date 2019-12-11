#ifndef FILEFORMATSTATEMACHINE_H
#define FILEFORMATSTATEMACHINE_H

/**
 * We have a custom data file format.
 * It's largely like a csv but with some additional information.
 * 
 * Line  1:          csvs of feature column types.
 * Line  2:          csvs of the possible class labels.
 * Line  3:          the string "@training"
 * Lines 4 to N:     csvs of the labeled training data (class label is the last column)
 * Line  N+1:        the string "@test"
 * Lines N+2 to EOF: csvs of the labeled test data
 */

#include <string>


class FileFormatStateMachine {
private:
    enum class State {
        READ_FEATURE_LINE,
        READ_CLASS_LABELS_LINE,
        READ_TRAINING_DELIM,
        READ_TRAINING_LINES,
        READ_TEST_LINES,
    };

    State current_state;
    bool error_flag;

public:
    // Used for the caller to know the action they should take based on the line
    enum class LineType {
        FEATURE_LINE,
        CLASS_LABELS_LINE,
        TRAINING_LINE,
        TEST_LINE,
        SKIP, // For the delimiter lines
    };

    FileFormatStateMachine();
    
    void reset() { current_state = State::READ_FEATURE_LINE; error_flag = false; }
    bool accepted() const { return !error_flag && current_state == State::READ_TEST_LINES; }
    LineType processLine(const std::string &line);
};


#endif
