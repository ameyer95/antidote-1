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

    SECTION("DataSet::summary sanity check") {
        REQUIRE(d1.summary() == (double)2/3);
        REQUIRE(d2.summary() == 0);
    }
}


TEST_CASE("DataSet operations") {
    bool bits[9] = {false, false, false,
                    false, true, true,
                    true, true, true};
    DataSet d(bits, 9, 3);
    Predicate phi0(0), phi1(1);
    vector<Predicate> phi_vector;
    phi_vector.push_back(phi0);
    phi_vector.push_back(phi1);
    PredicateSet predicates(&phi_vector);

    SECTION("Predicate evaluation check") {
        for(int row = 0; row < 3; row++) {
            REQUIRE(phi0.evaluate(d.getRow(row).first) == d.getRow(row).first[0]);
            REQUIRE(phi1.evaluate(d.getRow(row).first) == d.getRow(row).first[1]);
        }
    }

    SECTION("DataSet::impurity check") {
        double p = (double)2 / 3;
        REQUIRE(d.impurity() == p * (1-p) / 2);
    }

    // XXX Can't test the information gain calculation since it's currently private.
    // Could refactor for private use of public interface

    SECTION("PredicateSet::bestSplit check") {
        REQUIRE(predicates.bestSplit(d) == &phi_vector[1]);

        SECTION("DataSet::split check") {
            pair<DataSet*, DataSet*> split = d.split(phi1);
            REQUIRE(split.first->size() == 2);
            REQUIRE(split.first->summary() == 1);
            REQUIRE(split.second->size() == 1);
            REQUIRE(split.second->summary() == 0);
        }
    }
}
