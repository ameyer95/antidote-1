#include "catch.hpp"
#include "Feature.hpp"
#include "Predicate.hpp"

TEST_CASE("Check that Feature::operator =(...) updates appropriately") {
    const bool boolean_constant = true;
    const float float_constant = 0.1;
    Feature f;

    f = boolean_constant;
    REQUIRE(f.getType() == FeatureType::BOOLEAN);
    REQUIRE(f.getBooleanValue() == boolean_constant);

    f = float_constant;
    REQUIRE(f.getType() == FeatureType::NUMERIC);
    REQUIRE(f.getNumericValue() == float_constant);
}


TEST_CASE("Check that Predicates behave as expected on FeatureVectors") {
    // We'll build vectors (true, 0.5), (false, 0.3) in {0,1} x R
    FeatureVector x1(2);
    x1[0] = true;
    x1[1] = (float)0.5;
    FeatureVector x2(2);
    x2[0] = false;
    x2[1] = (float)0.3;

    Predicate phi0(0);
    Predicate phi1(1, 0.4); // Should check if x[1] <= 0.4

    REQUIRE(phi0.evaluate(x1) == true);
    REQUIRE(phi1.evaluate(x1) == false);
    REQUIRE(phi0.evaluate(x2) == false);
    REQUIRE(phi1.evaluate(x2) == true);
}
