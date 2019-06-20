#include "catch.hpp"
#include "information_math.h"
#include <utility>
using namespace std;

TEST_CASE("information_math computation sanity checks") {
    pair<int, int> whole(1, 2);
    pair<int, int> left(1, 1);
    pair<int, int> right(0, 1);

    REQUIRE(impurity(right) == 0);
    REQUIRE(impurity(left) == (double)1/8);
    REQUIRE(impurity(whole) < impurity(left));
    REQUIRE(informationGain(left, right) == impurity(whole) - (double)2/3 * impurity(left) - (double)1/3 * impurity(right));
}

//TODO test edge cases involving trivial splits, 0s, etc
