#include "catch.hpp"
#include "Interval.h"
#include <string>
using namespace std;

TEMPLATE_TEST_CASE("Testing to_string(Interval<T>)", "", int, double) {
    Interval<TestType> interval((TestType)1, (TestType)2);
    string expected_string = "[" + to_string((TestType)1) + ", " +
                             to_string((TestType)2) + "]";
    REQUIRE(to_string(interval) == expected_string);
}

TEMPLATE_TEST_CASE("Testing Interval<T> equality", "", int, double) {
    Interval<TestType> a((TestType)1, (TestType)2);
    Interval<TestType> b((TestType)1, (TestType)2);
    REQUIRE(a == b);
}

TEMPLATE_TEST_CASE("Testing Interval<T> arithmetic", "", int, double) {
    Interval<TestType> a(-(TestType)1, (TestType)2);
    Interval<TestType> b((TestType)3, (TestType)5);
    REQUIRE(-a == Interval<TestType>(-(TestType)2, (TestType)1));
    REQUIRE(a + b == Interval<TestType>((TestType)2, (TestType)7));
    REQUIRE(a - b == Interval<TestType>(-(TestType)6, -(TestType)1));
    REQUIRE(a * b == Interval<TestType>(-(TestType)5, (TestType)10));
    REQUIRE(a / b == Interval<TestType>(-(TestType)1/3, (TestType)2/3));
}
