#include "catch.hpp"
#include "CategoricalDistribution.hpp"
#include <vector>

TEST_CASE("Check CategoricalDistribution::operator [] appropriately wraps underlying vector") {
    CategoricalDistribution<float> p(3); // Should have 3 values
    std::vector<float> values = {.2, .5, .3};

    REQUIRE(p.size() == values.size());

    // Make sure basic assignment and accessing are consistent
    for(int i = 0; i < 3; i++) {
        p[i] = values[i];
        REQUIRE(p[i] == values[i]);
    }

    // Sanity check the iteration
    auto i = p.cbegin();
    auto j = values.cbegin();
    for( ; i != p.cend() && j != values.cend(); i++, j++) {
        REQUIRE(*i == *j);
    }
    REQUIRE(i == p.cend());
    REQUIRE(j == values.cend());
}
