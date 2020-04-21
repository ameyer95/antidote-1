#include "catch.hpp"
#include "ArffParser.h"
#include <string>
#include <iostream> 

using namespace std;

TEST_CASE("Check Arff Parser function for correct Arff file") {
    ArffParser* parser = new ArffParser("test/data/arff_test1.arff");
    DataSet* data = parser->parse(); 

    REQUIRE(data->feature_types.size() == 4); 
    REQUIRE(data->num_categories == 3); 
    REQUIRE(data->rows.size() == 10); 

    DataRow r0 = data->rows[0];
    REQUIRE(r0.y == 0); 
    REQUIRE(r0.x.at(0).getType() == FeatureType::NUMERIC); 
    REQUIRE(r0.x.at(0).getNumericValue() == Approx(5.1));

    DataRow r4 = data->rows[4];
    REQUIRE(r4.y == 1); 

    delete parser; 
    delete data; 

    parser = new ArffParser("test/data/arff_test_boolean.arff");
    data = parser->parse(); 

    REQUIRE(data->feature_types.size() == 6); 
    REQUIRE(data->rows.size() == 9);
    REQUIRE(data->feature_types.at(3) == FeatureType::BOOLEAN); 
    r0 = data->rows[0]; 
    REQUIRE(r0.x.at(3).getBooleanValue() == true);
} 