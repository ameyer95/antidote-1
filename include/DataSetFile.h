#ifndef DATASETFILE_H
#define DATASETFILE_H

#include "DataSet.hpp"
#include <string>
#include <map>


class DataSetFile {
private:
    FeatureVectorHeader header;
    std::vector<std::string> class_labels;
    std::vector<DataRow> training;
    std::vector<DataRow> test;

    std::map<std::string, int> label_map;

    // Private constructor as a form of error handling.
    // A public static method is used to get pointers to objects.
    DataSetFile();

    // These return true on successful parse.
    bool extractHeader(const std::string &line);
    bool extractClassLabels(const std::string &line);
    bool extractTrainingRow(const std::string &line);
    bool extractTestRow(const std::string &line);

    DataRow extractDataRow(const std::string &line);

public:
    // Returns NULL if there was an error, caller is responsible for deallocation
    static DataSetFile* loadFile(const std::string &filename);
    // TODO a method that generates the datareferences objects

    // DataSet stores the class as an int, so this converts between int and string labels
    const std::string& index_to_label(int i) const { return class_labels[i]; }
    int label_to_index(const std::string &label) const { return label_map.at(label); } // TODO could return -1 if not found
};


#endif
