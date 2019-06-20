#include "catch.hpp"
#include "data_common.h"
#include <vector>
using namespace std;

TEMPLATE_TEST_CASE("DataReferences<T> basic operations", "", int, vector<int>) {
    const int SIZE = 4;
    vector<TestType> raw_data(SIZE); // SIZE=4 uninitialized elements
    DataReferences<TestType> data_references(&raw_data);

    REQUIRE(data_references.size() == raw_data.size());

    SECTION("DataReferences<T>::operator [] returns correct objects") {
        for(int i = 0; i < data_references.size(); i++) {
            REQUIRE(&(data_references[i]) == &(raw_data[i]));
        }
    }

    SECTION("DataReferences<T>::remove affects only the references and is correct") {
        const int REMOVE_INDEX = 1;
        data_references.remove(REMOVE_INDEX);
        REQUIRE(data_references.size() == SIZE - 1);
        REQUIRE(raw_data.size() == SIZE);
        for(int i = 0; i < data_references.size(); i++) {
            int raw_index = (i >= REMOVE_INDEX ? i + 1 : i);
            REQUIRE(&(data_references[i]) == &(raw_data[raw_index]));
        }
    }
}
