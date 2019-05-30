#include "catch.hpp"
#include "DataSet.h"

TEST_CASE("DataSet object basics") {
    bool foo[3] = {true, true, false};
    bool bar[4] = {false, false, false, false};

    DataSet d1(foo, 3, 1);
    DataSet d2(bar, 4, 2);

    SECTION("Constructor builds vectors as expected") {
        REQUIRE(d1.size() == 3);
        for(int i = 0; i < d1.size(); i++) {
            REQUIRE(d1.getRow(i).first.size() == 0);
            REQUIRE(d1.getRow(i).second == foo[i]);
        }
        REQUIRE(d2.size() == 2);
        for(int i = 0; i < d2.size(); i++) {
            REQUIRE(d2.getRow(i).first.size() == 1);
            REQUIRE(d2.getRow(i).first[0] == bar[2*i]);
            REQUIRE(d2.getRow(i).second == bar[2*i+1]);
        }
    }

    SECTION("DataSet::countOnes sanity check") {
        REQUIRE(d1.countOnes() == 2);
        REQUIRE(d2.countOnes() == 0);
    }

    SECTION("DataSet::isPure sanity check") {
        REQUIRE(d1.isPure() == false);
        REQUIRE(d2.isPure() == true);
    }
}
