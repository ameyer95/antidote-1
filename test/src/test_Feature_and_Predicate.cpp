#include "catch.hpp"
#include "Feature.hpp"
#include "Predicate.hpp"
#include "SymbolicPredicate.hpp"
#include <optional>
using namespace std;

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
    FeatureVector x3(2);
    x3[0] = false;
    x3[1] = (float)0.4;

    SECTION("Checking Predicate") {
        Predicate phi0(0);
        Predicate phi1(1, 0.4); // Should check if x[1] <= 0.4
        Predicate phi2(1, 0.45);

        REQUIRE(phi0.evaluate(x1) == true);
        REQUIRE(phi1.evaluate(x1) == false);
        REQUIRE(phi2.evaluate(x1) == false);

        REQUIRE(phi0.evaluate(x2) == false);
        REQUIRE(phi1.evaluate(x2) == true);
        REQUIRE(phi2.evaluate(x2) == true);

        REQUIRE(phi0.evaluate(x3) == false);
        REQUIRE(phi1.evaluate(x3) == true);
        REQUIRE(phi2.evaluate(x3) == true);
    }

    SECTION("Checking SymbolicPredicate") {
        SymbolicPredicate phi0(0);
        SymbolicPredicate phi1(1, 0.3, 0.4); // Should check x[1] <= t for t in [0.3, 0.4)
        SymbolicPredicate phi2(1, 0.35, 0.45);

        REQUIRE(phi0.evaluate(x1) == optional<bool>(true));
        REQUIRE(phi1.evaluate(x1) == optional<bool>(false));
        REQUIRE(phi2.evaluate(x1) == optional<bool>(false));

        REQUIRE(phi0.evaluate(x2) == optional<bool>(false));
        REQUIRE(phi1.evaluate(x2) == optional<bool>(true));
        REQUIRE(phi2.evaluate(x2) == optional<bool>(true));

        REQUIRE(phi0.evaluate(x3) == optional<bool>(false));
        REQUIRE(phi1.evaluate(x3) == optional<bool>(false));
        // REQUIRE(phi2.evaluate(x3) == optional<bool>({}));
        REQUIRE(!phi2.evaluate(x3).has_value()); 
    }
}
