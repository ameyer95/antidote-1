#include "DataSetFile.h"
#include "FileFormatStateMachine.h"
#include <fstream>
#include <string>
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

DataSetFile* DataSetFile::loadFile(const string &filename) {
    ifstream infile;
    infile.open(filename, ios::in);
    // TODO
}

/**
 * Auxiliary definitions
 */
