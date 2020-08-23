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

//TODO test edge cases involving trivial splits, 0s, etc

TEST_CASE("test impurity function intervals") {
    vector<int> c1 = {1, 3};
    Interval<double> ret1 = impurity(c1, 1);
    
    REQUIRE(ret1.get_lower_bound() == 0.0);
    REQUIRE(ret1.get_upper_bound() == Approx((double)4.0/9));

    ret1 = impurity(c1, 2); 

    REQUIRE(ret1.get_lower_bound() == 0.0);
    REQUIRE(ret1.get_upper_bound() == Approx(0.5));

    ret1 = impurity(c1, 3); 

    REQUIRE(ret1.get_lower_bound() == 0.0);
    REQUIRE(ret1.get_upper_bound() == Approx(0.5));

    vector<int> c2 = {1, 2, 3};
    ret1 = impurity(c2, 2); 

    REQUIRE(ret1.get_lower_bound() == (double)3.0 / 16); // 0 + 0 + 3/16 
    REQUIRE(ret1.get_upper_bound() == (double)3.0 / 16 + 1.0 / 4 + 1.0 / 4); 

    ret1 = impurity(c2, 3); // across 0.5 

    REQUIRE(ret1.get_lower_bound() == 0.0); 
    REQUIRE(ret1.get_upper_bound() == (double)2.0 / 9 + 1.0 / 4 + 1.0 / 4); 

    ret1 = impurity(c2, 0); 

    REQUIRE(ret1.get_lower_bound() == Approx(ret1.get_upper_bound()));
}

TEST_CASE("test CategoricalDistribution intervals") {
    vector<int> c1 = {1, 3, 0, 0}; 
    CategoricalDistribution<Interval<double> > ret1 = estimateCategorical(c1, 1); 

    REQUIRE(ret1[0].get_lower_bound() == 0.0);
    REQUIRE(ret1[0].get_upper_bound() == Approx((double)1.0 / 3));

    REQUIRE(ret1[1].get_lower_bound() == Approx((double)2.0 / 3));
    REQUIRE(ret1[1].get_upper_bound() == 1.0); 

    ret1 = estimateCategorical(c1, 4); // count == dropout

    REQUIRE(ret1[0].get_lower_bound() == 0.0);
    REQUIRE(ret1[0].get_upper_bound() == 1.0);

    REQUIRE(ret1[1].get_lower_bound() == 0.0);
    REQUIRE(ret1[1].get_upper_bound() == 1.0);

    ret1 = estimateCategorical(c1, 5); // count < dropout

    REQUIRE(ret1[0].get_lower_bound() == 0.0);
    REQUIRE(ret1[0].get_upper_bound() == 1.0);

    REQUIRE(ret1[1].get_lower_bound() == 0.0);
    REQUIRE(ret1[1].get_upper_bound() == 1.0);

    vector<int> c2 = {1, 2, 3}; 
    ret1 = estimateCategorical(c2, 2); 

    REQUIRE(ret1[0].get_lower_bound() == 0);
    REQUIRE(ret1[1].get_lower_bound() == 0);
    REQUIRE(ret1[2].get_lower_bound() == (double)1.0 / 4);

    REQUIRE(ret1[0].get_upper_bound() == Approx((double)1.0 / 4));
    REQUIRE(ret1[1].get_upper_bound() == 0.5);
    REQUIRE(ret1[2].get_upper_bound() == 0.75);
}
