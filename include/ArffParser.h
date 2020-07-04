#ifndef ARFFPARSER_H
#define ARFFPARSER_H

#include <map>
#include <vector> 
#include <string> 

#include "ArffScanner.h"
#include "DataSet.hpp"
#include "ExperimentDataWrangler.h"
#include "Error.h"

using namespace std;

class ArffParser {
private: 
    void parseRelation(DataSet *data, bool booleanized, int label_ind);
    void parseData(DataSet *data, float thres);

    float stofloat(string s);
    vector<string> readNominal(string vals);
    map<string, int> makeNominalMap(vector<string> tags);
    string trimBrackets(string s); 

    ArffScanner* scanner;
    map<string, int> label_map;
    vector<string> labels;
    int label_id;
    map<int, map<string, int> > boolean_maps; // stores mapping info for boolean attributes (index + val = boolean val)
    set<int> ignored_inds; // indices with nominal data that is not used 
    Error* err_handler; 

public: 
    ArffParser(const string& _file);
    ~ArffParser(); 
    DataSet* parse(float thres = 0.0, int label_ind = -1);
    // copies and returns copy of labels. 
    vector<string> getLabels(); 
    static ExperimentData* loadArff(std::string train_path, std::string test_path, bool booleanized = false, float thres = 0, int label_ind = -1); 
    bool isFatal(); 
    bool isWarning(); 
};

#endif // ARFFPARSER_H
