#ifndef UCI_H
#define UCI_H

#include <set>
#include <string>
#include <vector>

/**
 * Support for reading in the UCI csv data files.
 * For now we assume no missing values, all-float features, and string labels.
 * The file reading functions make strong assumptions
 * that the files look as expected.
 */

enum class UCINames { IRIS, CANCER, WINE };

struct UCIDetails {
    std::string file_name;
    int num_cols; // Including indices_to_ignore and the class label
    int label_index; // Counting from 0
    std::vector<int> indices_to_ignore; // Since some have a unique patient id, etc
    int num_rows;
};

const UCIDetails UCI_IRIS_DETAILS = { "bezdekIris.data", 5, 4, {}, 150 };
const UCIDetails UCI_CANCER_DETAILS = { "wdbc.data", 32, 1, {0}, 569 };
const UCIDetails UCI_WINE_DETAILS = { "wine.data", 14, 0, {}, 178 };

struct CSVRow {
    std::vector<float> x;
    std::string y;
};


class UCI {
private:
    const UCIDetails *details;
    std::vector<CSVRow> data;
    std::set<std::string> labels;

    void setDetails(const UCINames &name);
    void loadFromFile(const std::string &prefix);
    void parseLine(CSVRow &csv_row, const std::string &comma_separated_line);
    
public:
    UCI(const UCINames &name, const std::string &prefix);

    const std::set<std::string>& getLabels() { return labels; }
    const std::vector<CSVRow>& getData() { return data; }
};


#endif
