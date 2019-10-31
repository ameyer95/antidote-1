#include "catch.hpp"
#include "DataReferences.h"
#include "DataSet.hpp"
#include "Feature.hpp"
#include <vector>
using namespace std;

TEST_CASE("DataReferences<T> basic operations") {
    const int NUM_FEATURES = 3;
    const int NUM_ROWS = 4;
    FeatureVectorHeader header(NUM_FEATURES, FeatureType::BOOLEAN);
    vector<DataRow> rows(NUM_ROWS); // All uninitialized elements
    DataSet data_set = { header, 2, rows };

    DataReferences data_references(&data_set);

    REQUIRE(data_references.size() == data_set.rows.size());

    SECTION("DataReferences<T>::operator [] returns correct objects") {
        for(unsigned int i = 0; i < data_references.size(); i++) {
            REQUIRE(&(data_references[i]) == &(data_set.rows[i]));
        }
    }

    SECTION("DataReferences<T>::remove affects only the references and is correct") {
        const unsigned int REMOVE_INDEX = 1;
        data_references.remove(REMOVE_INDEX);
        REQUIRE(data_references.size() == NUM_ROWS - 1);
        REQUIRE(data_set.rows.size() == NUM_ROWS);
        for(unsigned int i = 0; i < data_references.size(); i++) {
            unsigned int raw_index = (i >= REMOVE_INDEX ? i + 1 : i);
            REQUIRE(&(data_references[i]) == &(data_set.rows[raw_index]));
        }
    }
}
