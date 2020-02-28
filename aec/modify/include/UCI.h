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

enum class UCINames { IRIS, CANCER, WINE, YEAST, RETINOPATHY, MAMMOGRAPHY, FOO };

struct UCIDetails {
    std::string training_file_name;
    std::string test_file_name;
    int num_cols; // Including indices_to_ignore and the class label
    int label_index; // Counting from 0
    std::vector<int> indices_to_ignore; // Since some have a unique patient id, etc
    int num_rows;
};

const UCIDetails UCI_IRIS_DETAILS = { "iris80.data", "iris20.data", 5, 4, {}, 150 };
const UCIDetails UCI_CANCER_DETAILS = { "wdbc80.data", "wdbc20.data", 32, 1, {0}, 569 };
const UCIDetails UCI_WINE_DETAILS = { "winequality80.data", "winequality20.data", 12, 11, {}, 6497 };
const UCIDetails UCI_YEAST_DETAILS = { "yeast80.data", "yeast20.data", 10, 9, {0}, 1484 };
const UCIDetails UCI_RETINOPATHY_DETAILS = { "retinopathy80.data", "retinopathy20.data", 20, 19, {}, 1151 };
const UCIDetails UCI_MAMMOGRAPHY_DETAILS = { "mammographic_masses80.data", "mammographic_masses20.data", 6, 5, {}, 830 };
const UCIDetails UCI_FOO_DETAILS = { "foo-train.csv", "foo-test.csv", 3, 2, {}, 40 };

struct CSVRow {
    std::vector<float> x;
    std::string y;
};


class UCI {
private:
    const UCIDetails *details;
    std::vector<CSVRow> training_data;
    std::vector<CSVRow> test_data;
    std::set<std::string> labels;

    void setDetails(const UCINames &name);
    void loadFromFile(const std::string &filepath, std::vector<CSVRow> *data);
    void parseLine(CSVRow &csv_row, const std::string &comma_separated_line);
    
public:
    UCI(const UCINames &name, const std::string &prefix);

    const std::set<std::string>& getLabels() { return labels; }
    const std::vector<CSVRow>& getTrainingData() { return training_data; }
    const std::vector<CSVRow>& getTestData() { return test_data; }
};


#endif
