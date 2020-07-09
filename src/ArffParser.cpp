#include "ArffParser.h"

using namespace std; 

/**
 * checks if two strings are the same (case insensitive)
 */
inline bool scompare(string a, string b) {
    if (a.length() != b.length()) 
        return false;
    for (int i = 0; i < a.length(); i++) {
        if (tolower(a.at(i)) != tolower(b.at(i)))
            return false;
    }
    return true;
}

/**
 * reads nominal data declaration in the form of <val1>,<val2>,...,<valn>
 * and returns a vector of values 
 */
vector<string> ArffParser::readNominal(string vals) {
    istringstream iss(vals);
    vector<string> tags = vector<string>(); 
    string tag; 
    while (getline(iss, tag, ',')) {
        tags.push_back(tag);
    }
    return tags; 
}

/**
 * converts a vector of values to a map of value to its index 
 */
map<string, int> ArffParser::makeNominalMap(vector<string> tags) {
    map<string, int> tag_map; 
    for (int i = 0; i < tags.size(); i++) {
        tag_map.emplace(tags[i], i); 
    }
    return tag_map; 
}

/**
 * converts string to float
 * (added customized error checking and trimming compared to stock stof)
 */
float ArffParser::stofloat(string s) {
    std::istringstream iss(s);
    float f;
    iss >> noskipws >> f; // noskipws considers leading whitespace invalid
    // Check the entire string was consumed and if either failbit or badbit is set
    if (iss.eof() && !iss.fail()) {
        return f;
    } else {
        err_handler->warning("Invalid number: " + s);
        return f;
    }
}

/**
 * trims brackets from nominal attribute declaration 
 */
string ArffParser::trimBrackets(string s) {
    if (s.find("{") == string::npos) {
        if (s.find("}") == string::npos) {
            err_handler->fatal("Unsupported Attribute line"); 
        } else {
            err_handler->fatal("Incorrect class or label declaration"); 
        } 
    } 
    if (s.find("}") == string::npos) {
        err_handler->fatal("Incorrect class or label declaration"); 
    }

    s.erase(0, s.find("{") + 1);
    s.erase(s.find("}"));
    return s;
}

ArffParser::ArffParser(const string& _file) {
    label_id = 0;
    scanner = new ArffScanner(_file);
    err_handler = new Error(); 
}

ArffParser::~ArffParser() {
    if (scanner != NULL)
        delete scanner; 
    delete err_handler;
}

/**
 * parse a arff file passed as constructor parameter, returns NULL on fail 
 */
DataSet* ArffParser::parse(float thres, int label_ind) {
    DataSet* data = new DataSet();
    parseRelation(data, thres > 0, label_ind);
    if (isFatal()) {
        delete data; 
        return NULL; 
    }
    parseData(data, thres);
    if (isFatal()) {
        delete data; 
        return NULL; 
    } else {
        return data;
    }
}

/**
 * Parse the part begins with @RELATION in the arff file, adds FeatureType information to the Dataset,
 * with respect to the booleanized argument
 * 
 * @param data pointer to the output dataset
 * @param booleanized booleanize numeric attributes 
 * @param label_ind index of attribute to be treated as label 
 */
void ArffParser::parseRelation(DataSet *data, bool booleanized, int label_ind) {
    string cur; 
    int attrNum = 0;
    label_id = label_ind;
    scanner->nextLine(); 
    if (!scompare(scanner->nextWord(), "@RELATION")) {
        err_handler->fatal("Incorrect header (\"@RELATION\" declaration not found)");
    }

    // read attribute declarations 
    while (true) {
        scanner->nextLine(); 
        cur = scanner->nextWord(); 
        if (scompare(cur, "@DATA")) {
            break;
        }
        if (!scompare(cur, "@ATTRIBUTE")) {
            err_handler->fatal("Incorrect header (line begin with token other than \"@DATA\" or \"@ATTRIBUTE\")"); 
        }
        cur = scanner->nextWord(); // ignore the attribute name
        cur = scanner->nextWord(); 
        if (scompare(cur, "NUMERIC")) {
            if (booleanized)
                data->feature_types.push_back(FeatureType::BOOLEAN);
            else
                data->feature_types.push_back(FeatureType::NUMERIC);
        } else {    // attempt to parse as label or boolean line 
            string tmp = scanner->curLine;
            tmp = trimBrackets(tmp); 
            vector<string> tags = readNominal(tmp);

            if (tags.size() > 2) { // treat as label
                if (label_id != -1 && attrNum != label_id) {
                    // ignore label 
                    ignored_inds.emplace(attrNum); 
                } else {
                    label_id = attrNum;
                    label_map = makeNominalMap(tags); 
                    labels = tags; 
                }
            } else {// treat as boolean 
                data->feature_types.push_back(FeatureType::BOOLEAN);
                boolean_maps.emplace(attrNum, makeNominalMap(tags));
            }
        }
        attrNum++;
    }
    data->num_categories = labels.size();
}

/**
 * parse the @DATA section of the arff file, ignores lines with error 
 * 
 * @param data pointer to the output dataset
 * @param thres threshold to booleanize numeric attributes 
 */ 
void ArffParser::parseData(DataSet *data, float thres) {
    while (!scanner->nextLine()) {
        DataRow curRow; 
        FeatureVector vals(data->feature_types.size()); 
        string val; 
        int attrNum = 0; // index of current reading attribute (DOES NOT INCLUDE LABEL!)
        int read_label = 0;
        int ignored_attrs = 0; // number of ignored nominal attributes 
        while (getline(scanner->iss, val, ',')) {
            if ((attrNum == data->feature_types.size()) && (attrNum != label_id)) {
                // reaches the end but do not have label 
                err_handler->warning("Incorrect data line"); 
                break;
            }
            if (attrNum == label_id && (!read_label)) {
                // read label info 
                if (label_map.count(val) == 0) {
                    err_handler->warning("Invalid label: " + val);
                } else {
                    curRow.y = label_map[val];
                }
                read_label = 1;
            } else {
                // reading attribute 
                if (ignored_inds.count(attrNum + read_label)) {
                    // ignored (undefined nominal attribute)
                    ignored_attrs++;
                } else if (boolean_maps.count(attrNum)) {    // interpret as boolean
                    if (!boolean_maps[attrNum].count(val)) {
                        err_handler->warning("invalid binary value: " + val);
                    }
                    vals[attrNum - ignored_attrs] = (bool)boolean_maps[attrNum][val];
                } else {    // interpret as float 
                    vals[attrNum - ignored_attrs] = stofloat(val);
                    if (thres) { // booleanize data
                        vals[attrNum - ignored_attrs] = vals[attrNum - ignored_attrs].getNumericValue() > thres;
                    }
                }
                attrNum++;
            }
        }
        if (attrNum - ignored_attrs != data->feature_types.size()) { // insufficient attributes read 
            err_handler->warning("Incorrect data line, should have " + to_string(data->feature_types.size()) + " attributes, but only read " + to_string(attrNum - ignored_attrs));
        }
        curRow.x = vals;
        if (err_handler->isWarning()) { // ignore problematic data row 
            err_handler->resetWarning(); 
        } else {
            data->rows.push_back(curRow);
        }
    }
}

/**
 * returns vector of labels of this arff file 
 */
vector<string> ArffParser::getLabels() {
    vector<string> ret = labels; 
    return ret;  
}

/**
 * returns ExperimentData object with specified train_path and test_path 
 * 
 * @param train_path path to the trainning set 
 * @param test_path path to the test set 
 * @param booleanized booleanize numeric attributes (default to false)
 * @param thres threshold to booleanize numeric attributes (used together with booleanized)
 * @param label_ind index to be treated as label (default to the -1:the last attribute)
 */
ExperimentData* ArffParser::loadArff(std::string train_path, std::string test_path, bool booleanized, float thres, int label_ind) {
    ArffParser train_parser(train_path);
    ArffParser test_parser(test_path);
    Error* err_handler = new Error(); 
    DataSet *train_dat, *test_dat;
    train_dat = train_parser.parse(booleanized ? thres : 0, label_ind);
    test_dat = test_parser.parse(booleanized ? thres : 0, label_ind);
    // checks for label consistency 
    std::vector<std::string> train_label = train_parser.getLabels(); 
    std::vector<std::string> test_label = test_parser.getLabels(); 
    if (train_label.size() != test_label.size()) {
        err_handler->fatal("Mismatch between train and test labels");
    }
    for (int i = 0; i < train_label.size(); i++) {
        if (train_label[i].compare(test_label[i]) != 0) {
            err_handler->fatal("Mismatch between train and test labels");
        }
    }
    if (err_handler->isFatal()) {
        delete err_handler; 
        return NULL; 
    }
    ExperimentData *ret = new ExperimentData { train_dat, test_dat, train_parser.getLabels() };
    delete err_handler;
    return ret;
}

void ArffParser::writeArff(DataSet *data, string output_path, const vector<string>* class_labels) {
    Error* err_handler = new Error(); 
    bool use_label = (class_labels != NULL);
    ofstream of(output_path, ofstream::out); 
    if(!of) {
        err_handler->fatal("Cannot open output file!"); 
        delete err_handler; 
        return; 
    }
    // write @RELATION part 
    of << "@RELATION Experiment_data\n\n"; 
    for (int i = 0; i < data->feature_types.size(); i++) {
        of << "@ATTRIBUTE attr" << i << "\t"; 
        if (data->feature_types[i] == FeatureType::NUMERIC)
            of << "NUMERIC\n"; 
        else if (data->feature_types[i] == FeatureType::BOOLEAN)
            of << "{false,true}\n";
    }
    of << "@ATTRIBUTE label\t{";
    for (int i = 0; i < data->num_categories; i++) {
        if (i > 0) 
            of << ",";
        if (use_label) 
            of << class_labels->at(i);
        else 
            of << i;
    }
    of << "}\n"; 

    // write @DATA part 
    of << "\n@DATA\n"; 
    for (int i = 0; i < data->rows.size(); i++) {
        for (int j = 0; j < data->rows[i].x.size(); j++) {
            if (data->feature_types[j] == FeatureType::NUMERIC) {
                of << data->rows[i].x[j].getNumericValue(); 
            } else if (data->feature_types[j] == FeatureType::BOOLEAN) {
                of << (data->rows[i].x[j].getBooleanValue() ? "true" : "false");
            }
            of << ",";
        }
        if(use_label) 
            of << class_labels->at(data->rows[i].y) << "\n";
        else 
            of << data->rows[i].y << "\n"; 
    }
    of.close(); 
    delete err_handler;
    return;
}

bool ArffParser::isFatal() {
    return err_handler->isFatal();
}

bool ArffParser::isWarning() {
    return err_handler->isWarning();
}
