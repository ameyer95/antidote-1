#include "catch.hpp"
#include "information_math.h"
#include <utility>
using namespace std;

TEST_CASE("information_math computation sanity checks") {
    BinarySamples whole = {1, 2}, left = {1, 1}, right = {0, 1};

    REQUIRE(impurity(right) == 0);
    REQUIRE(impurity(left) == (double)1/8);
    REQUIRE(impurity(whole) < impurity(left));
    REQUIRE(jointImpurity(left, right) <= impurity(whole) * (whole.num_zeros + whole.num_ones));
}

TEST_CASE("information_math labels") {
    BinarySamples whole = {1,2}, left = {1,1}, right = {0,1};

    
    REQUIRE(impurityLabels(right) == 0);
    REQUIRE(impurityLabels(left) == (double)1/8);
    REQUIRE(impurityLabels(whole) < impurityLabels(left));
    REQUIRE(jointImpurityLabels(left, right) <= impurityLabels(whole) * (whole.num_zeros + whole.num_ones));
}
//TODO test edge cases involving trivial splits, 0s, etc
