#include "UCI.h"
#include <algorithm>
#include <fstream>
#include <set>
#include <sstream>
#include <string>
#include <vector>
using namespace std;

UCI::UCI(const UCINames &name, const string &prefix) {
    setDetails(name);
    loadFromFile(prefix + "/" + details->training_file_name, &training_data);
    loadFromFile(prefix + "/" + details->test_file_name, &test_data);
}

void UCI::setDetails(const UCINames &name) {
    switch(name) {
        case UCINames::IRIS:
            details = &UCI_IRIS_DETAILS;
            break;
        case UCINames::CANCER:
            details = &UCI_CANCER_DETAILS;
            break;
        case UCINames::WINE:
            details = &UCI_WINE_DETAILS;
            break;
        case UCINames::YEAST:
            details = &UCI_YEAST_DETAILS;
            break;
        case UCINames::RETINOPATHY:
            details = &UCI_RETINOPATHY_DETAILS;
            break;
    }
}

void UCI::loadFromFile(const std::string &filepath, std::vector<CSVRow> *data) {
    ifstream file;
    file.open(filepath);
    string line;
    // XXX we just stop reading after we run out of lines
    // without checking if it matches the expected details
    // (which maybe are unnecessary?)
    while(getline(file, line)) {
        CSVRow temp;
        parseLine(temp, line); // Handles accruing the output class labels, too
        data->push_back(temp);
    }
    file.close();
}

void UCI::parseLine(CSVRow &csv_row, const std::string &comma_separated_line) {
    vector<string> items;
    istringstream iss(comma_separated_line);
    string item;
    while(getline(iss, item, ',')) {
        items.push_back(item);
    }
    // XXX assuming that items.size() is consistent with details->num_cols
    csv_row.x = vector<float>(0);
    for(int i = 0; i < details->num_cols; i++) {
        if(std::none_of(details->indices_to_ignore.cbegin(), details->indices_to_ignore.cend(), [i](int j){ return i == j; })) {
            if(i == details->label_index) {
                csv_row.y = items[i];
                labels.insert(items[i]);
            } else {
                csv_row.x.push_back(stof(items[i]));
            }
        }
    }
}
