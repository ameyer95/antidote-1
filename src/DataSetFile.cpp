#include "DataSetFile.h"
#include "DataSet.hpp"
#include "FileFormatStateMachine.h"
#include "string_common.h"
#include <fstream>
#include <map>
#include <string>
#include <utility>
#include <vector>
using namespace std;

/**
 * Auxiliary declarations
 */


/**
 * DatasetFile methods
 */

DataSetFile::DataSetFile() {
    // TODO
}

// Strings in the header line must be either "bool" or "real" or parsing will fail
bool DataSetFile::extractHeader(const string &line) {
    vector<string> fields;
    commaSplit(line, fields, true);
    header = FeatureVectorHeader(fields.size());
    for(int i = 0; i < fields.size(); i++) {
        if(fields[i] == "bool") {
            header[i] = FeatureType::BOOLEAN;
        } else if(fields[i] == "real") {
            header[i] = FeatureType::NUMERIC;
        } else {
            return false;
        }
    }
    return true;
}

bool DataSetFile::extractClassLabels(const string &line) {
    commaSplit(line, class_labels, true);
    for(int i = 0; i < class_labels.size(); i++) {
        label_map.insert(make_pair(class_labels[i], i));
    }
    return true; // TODO could ensure that there are no empty or duplicate labels
}

bool DataSetFile::extractTrainingRow(const string &line) {
    return extractDataRow(line, &training);
}

bool DataSetFile::extractTestRow(const string &line) {
    return extractDataRow(line, &test);
}

bool DataSetFile::extractDataRow(const string &line, vector<DataRow> *store) {
    vector<string> fields;
    commaSplit(line, fields, true);
    // TODO
    // make sure that the length agrees with the header
    // convert all of the features to the Feature type
    // convert the class string to the appropriate int from the header
}

DataSetFile* DataSetFile::loadFile(const string &filename) {
    typedef FileFormatStateMachine::LineType Action;

    DataSetFile *ret = new DataSetFile();
    ifstream infile;
    string line;
    FileFormatStateMachine m;
    bool error_flag = false;

    infile.open(filename, ios::in);
    while(getline(infile, line)) {
        Action action = m.processLine(line);
        switch(action) {
            case Action::FEATURE_LINE:
                error_flag = !ret->extractHeader(line);
                break;
            case Action::CLASS_LABELS_LINE:
                error_flag = !ret->extractClassLabels(line);
                break;
            case Action::TRAINING_LINE:
                error_flag = !ret->extractTrainingRow(line);
                break;
            case Action::TEST_LINE:
                error_flag = !ret->extractTestRow(line);
                break;
            case Action::SKIP:
                break;
        }
        if(error_flag) {
            break;
        }
    }
    infile.close();
    error_flag = error_flag || !m.accepted(); // Either an error in parsing a row or in the file format FSM

    if(error_flag) { // There was some error
        delete ret;
        return NULL;
    } else {
        return ret;
    }
}


/**
 * Auxiliary definitions
 */
