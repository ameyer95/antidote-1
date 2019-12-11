#ifndef DATASETFILE_H
#define DATASETFILE_H

#include "DataSet.hpp"
#include <string>

class DataSetFile {
private:
    FeatureVectorHeader header;
    std::vector<std::string> class_labels;
    std::vector<DataRow> training;
    std::vector<DataRow> test;

    // Private constructor as a form of error handling.
    // A public static method is used to get pointers to objects.
    DataSetFile();

public:
    static DataSetFile* loadFile(const std::string &filename);
    // TODO a method that generates the datareferences objects
};

#endif
