#ifndef ARFFPARSER_H
#define ARFFPARSER_H

#include <map>

#include "ArffScanner.h"
#include "DataSet.hpp"
#include "ExperimentDataWrangler.h"

using namespace std;

class ArffParser {
private: 
    void parseRelation(DataSet *data, bool booleanized = false);
    void parseData(DataSet *data, float thres);

    ArffScanner* scanner;
    map<string, int> label_map;
    vector<string> labels;
    int label_id;

public: 
    ArffParser(const string& _file);
    ~ArffParser(); 
    DataSet* parse(float thres = 0);
    // copies and returns copy of labels. 
    vector<string> getLabels(); 
    static ExperimentData* loadArff(std::string train_path, std::string test_path, bool booleanized = false, float thres = 0); 
};

#endif // ARFFPARSER_H
