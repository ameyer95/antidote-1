#include "catch.hpp"
#include "ArffParser.h"
#include <string>
using namespace std;

TEST_CASE("Check Arff Parser function for incorrect Arff file") {
    ArffParser* parser = new ArffParser("test/data/arff_test2(bad_lines).arff");
    DataSet* data = parser->parse(); 

    REQUIRE(data->feature_types.size() == 4); 
    REQUIRE(data->num_categories == 3); 
    REQUIRE(data->rows.size() == 7); 

    delete parser; 

    parser = new ArffParser("test/data/arff_test3(no_relation).arff");
    delete data; 
    data = parser->parse();

    REQUIRE(data == NULL);
    REQUIRE(parser->isFatal());
    delete parser; 
}