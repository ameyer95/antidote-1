#include <string>

#include "ArffParser.h"

using namespace std; 

inline bool scompare(string a, string b) {
    if(a.length() != b.length()) 
        return false;
    for(int i = 0; i < a.length(); i++) {
        if(tolower(a.at(i)) != tolower(b.at(i)))
            return false;
    }
    return true;
}

float ArffParser::stofloat(string s) {
    std::istringstream iss(s);
    float f;
    iss >> noskipws >> f; // noskipws considers leading whitespace invalid
    // Check the entire string was consumed and if either failbit or badbit is set
    if(iss.eof() && !iss.fail()) {
        return f;
    } else {
        err_handler->warning("Invalid number");
        return f;
    }
}

ArffParser::ArffParser(const string& _file): scanner(NULL),
                                             label_id(0) {
    scanner = new ArffScanner(_file);
    labels = vector<string>(); 
    label_map = map<string, int>();
    err_handler = new Error(); 
}

ArffParser::~ArffParser() {
    if(scanner != NULL)
        delete scanner; 
    delete err_handler;
}

DataSet* ArffParser::parse(float thres) {
    DataSet* data = new DataSet();
    parseRelation(data, thres > 0);
    if(isFatal()) {
        delete data; 
        return NULL; 
    }
    parseData(data, thres);
    if(isFatal()) {
        delete data; 
        return NULL; 
    } else {
        return data;
    }
}

void ArffParser::parseRelation(DataSet *data, bool booleanized) {
    string cur; 
    int lineNum = 0;
    scanner->nextLine(); 
    if(!scompare(scanner->nextWord(), "@RELATION")) {
        err_handler->fatal("Incorrect header (\"@RELATION\" declaration not found)");
    }
    while(true) {
        scanner->nextLine(); 
        cur = scanner->nextWord(); 
        if(scompare(cur, "@DATA")) {
            break;
        }
        if(!scompare(cur, "@ATTRIBUTE")) {
            err_handler->fatal("Incorrect header (line begin with token other than \"@DATA\" or \"@ATTRIBUTE\")"); 
        }
        cur = scanner->nextWord(); 
        if(cur.compare("class") == 0 || cur.compare("label") == 0) {
            if(label_id) {
                err_handler->fatal("Multiple class or label declaration");
            }
            label_id = lineNum;
            string tmp = scanner->curLine;
            if(tmp.find("{") == string::npos || tmp.find("}") == string::npos) {
                err_handler->fatal("Incorrect class or label declaration"); 
            }
            tmp.erase(0, tmp.find("{") + 1);
            tmp.erase(tmp.find("}"));
            istringstream iss(tmp);
            string label; 
            while(getline(iss, label, ',')) {
                labels.push_back(label);
                label_map.emplace(label, labels.size() - 1);
            }
        }
        cur = scanner->nextWord(); 
        if(scompare(cur, "NUMERIC")) {
            if(booleanized)
                data->feature_types.push_back(FeatureType::BOOLEAN);
            else
                data->feature_types.push_back(FeatureType::NUMERIC);
        }
        lineNum++;
    }
    data->num_categories = labels.size();
}

void ArffParser::parseData(DataSet *data, float thres) {
    while(!scanner->nextLine()) {
        DataRow curRow; 
        FeatureVector vals(data->feature_types.size()); 
        string val; 
        int id = 0;
        while(getline(scanner->iss, val, ',')) {
            if(id == data->feature_types.size() && id != label_id) {
                err_handler->warning("Incorrect data line"); 
                break;
            }
            if(id == label_id) {
                if(label_map.count(val) == 0) {
                    err_handler->warning("Invalid label");
                } else {
                    curRow.y = label_map[val];
                }
            } else {
                vals[id] = stofloat(val);
                if(thres) {
                    vals[id] = vals[id].getNumericValue() > thres;
                }
                id++;
            }
        }
        if(id != data->feature_types.size()) {
            err_handler->warning("Incorrect data line");
        }
        curRow.x = vals;
        if(err_handler->isWarning()) {
            err_handler->resetWarning(); 
        } else {
            data->rows.push_back(curRow);
        }
    }
}

vector<string> ArffParser::getLabels() {
    vector<string> ret = labels; 
    return ret;  
}

ExperimentData* ArffParser::loadArff(std::string train_path, std::string test_path, bool booleanized, float thres) {
    ArffParser train_parser(train_path);
    ArffParser test_parser(test_path);
    Error* err_handler = new Error(); 
    DataSet *train_dat, *test_dat;
    train_dat = train_parser.parse(booleanized ? thres : 0);
    test_dat = test_parser.parse(booleanized ? thres : 0);
    // checks for label consistency 
    std::vector<std::string> train_label = train_parser.getLabels(); 
    std::vector<std::string> test_label = test_parser.getLabels(); 
    if(train_label.size() != test_label.size()) {
        err_handler->fatal("Mismatch between train and test labels");
    }
    for(int i = 0; i < train_label.size(); i++) {
        if(train_label[i].compare(test_label[i]) != 0) {
            err_handler->fatal("Mismatch between train and test labels");
        }
    }
    if(err_handler->isFatal()) {
        return NULL; 
    }
    ExperimentData *ret = new ExperimentData { train_dat, test_dat, train_parser.getLabels() };
    return ret;
}

bool ArffParser::isFatal() {
    return err_handler->isFatal();
}

bool ArffParser::isWarning() {
    return err_handler->isWarning();
}
