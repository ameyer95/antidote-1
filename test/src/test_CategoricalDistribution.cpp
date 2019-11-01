#include "catch.hpp"
#include "CategoricalDistribution.hpp"
#include <vector>

TEST_CASE("Check CategoricalDistribution::operator [] appropriately wraps underlying vector") {
    CategoricalDistribution<float> p(3); // Should have 3 values
    std::vector<float> values = {.2, .5, .3};

    // Make sure basic assignment and accessing are consistent
    for(int i = 0; i < 3; i++) {
        p[i] = values[i];
        REQUIRE(p[i] == values[i]);
    }

    REQUIRE(p.size() == values.size());
}
