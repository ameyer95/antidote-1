#include "catch.hpp"
#include "ArffParser.h"
#include "DropoutDomains.hpp"
#include <string>
#include <iostream> 

using namespace std;

// Peripheral tests 
TEST_CASE("Check meet impurity operations of TrainingReferencesWithDropout") {
    TrainingSetDropoutDomain domain; 
    ArffParser* parser = new ArffParser("test/data/impurity_test.arff"); 
    DataSet* data = parser->parse(); 
    // 9w, 4b
    DataReferences d1(data); 

    TrainingReferencesWithDropout set1(d1, 4); 
    TrainingReferencesWithDropout set_pure = domain.meetImpurityEqualsZero(set1); 
    TrainingReferencesWithDropout set_notpure = domain.meetImpurityNotEqualsZero(set1); 

    vector<int> counts = set_pure.baseCounts(); 
    REQUIRE(counts[0] == 0); 
    REQUIRE(counts[1] == 9);

    counts = set_notpure.baseCounts(); 
    REQUIRE(counts[0] == 4); 
    REQUIRE(counts[1] == 9); // action not performed 

    DataReferences d2 = d1;
    TrainingReferencesWithDropout set2(d2, 2); 
    set_pure = domain.meetImpurityEqualsZero(set2); 

    REQUIRE(domain.isBottomElement(set_pure));
}

TEST_CASE("Check join operation of TrainingReferencesWithDropout") {
    ArffParser* parser = new ArffParser("test/data/arff_test1.arff");
    DataSet* data = parser->parse(); 
    int num_dropout = 2; 
    DataReferences data_ref(data); 
    TrainingReferencesWithDropout set1(data_ref, num_dropout);
    TrainingReferencesWithDropout set2(data_ref, num_dropout + 1);
    TrainingSetDropoutDomain domain; 
    TrainingReferencesWithDropout set_join = domain.binary_join(set1, set2);
    REQUIRE(set_join.num_dropout == 3);
    delete parser;
    parser = new ArffParser("test/data/arff_test1.arff");
    DataSet* data_dup = parser->parse(); 
    data_dup->rows.erase(data_dup->rows.begin());
    DataReferences data_ref2(data_dup); 
    TrainingReferencesWithDropout set3(data_ref2, num_dropout);
    set_join = domain.binary_join(set1, set3);
    REQUIRE(set_join.num_dropout == 3);
    delete parser; 
} 

TEST_CASE("Check PredicateSetDomain") {
    PredicateSetDomain domain; 

    PredicateAbstraction phis; 
    phis.push_back(optional<SymbolicPredicate>()); 

    PredicateAbstraction res = domain.meetPhiIsBottom(phis); 
    REQUIRE(res.size() == 1); 

    phis[0] = SymbolicPredicate(0); 
    REQUIRE(phis[0].has_value());
    res = domain.meetPhiIsBottom(phis); 
    REQUIRE(res.size() == 0); 

    FeatureVector x1; 
    Feature f1;
    f1 = true; 
    x1.push_back(f1); 
    res = domain.meetXModelsPhi(phis, x1);
    REQUIRE(res.size() == 1);

    x1[0] = false; 
    res = domain.meetXNotModelsPhi(phis, x1);
    REQUIRE(res.size() == 1);
}

TEST_CASE("Check PosteriorDistributionIntervalDomain") {
    // Does not test Interval implementation (see test_Interval.cpp)
    PosteriorDistributionIntervalDomain domain; 

    PosteriorDistributionAbstraction dist1; 
    REQUIRE(domain.isBottomElement(dist1) == true); 

    PosteriorDistributionAbstraction dist2(3); 
    REQUIRE(domain.isBottomElement(dist2) == false); 

    dist2[1] = Interval<double>(1.0, 2.0); 
    PosteriorDistributionAbstraction dist3 = domain.binary_join(dist1, dist2); 
    REQUIRE(dist3.size() == 3); 
}

// Real BoxDropoutDomain tests
TEST_CASE("Test BoxDropoutDomain bestSplit") {
    DropoutDomains d; 
    BoxDropoutDomain domain = d.box_domain; 
    ArffParser* parser = new ArffParser("test/data/example_split.arff"); 
    DataSet* data = parser->parse(); 

    DataReferences ref(data); 
    TrainingReferencesWithDropout set1(ref, 1);
    PredicateAbstraction p1 = domain.bestSplit(set1); 

    REQUIRE(p1.size() == 2);
}

TEST_CASE("Test BoxDropoutDomain filter and summary") {
    DropoutDomains d; 
    BoxDropoutDomain domain = d.box_domain; 
    ArffParser* parser = new ArffParser("test/data/example_split.arff"); 
    DataSet* data = parser->parse(); 

    DataReferences ref(data); 
    TrainingReferencesWithDropout set1(ref, 1);
    PredicateAbstraction p1 = domain.bestSplit(set1); 
    TrainingReferencesWithDropout set2 = domain.filter(set1, p1); 

    REQUIRE(set2.baseCounts()[0] == 2);
    REQUIRE(set2.baseCounts()[1] == 7);
    REQUIRE(set2.num_dropout == 4);
    // cout << set2.baseCounts()[0] << " " << set2.baseCounts()[1] << " " << set2.num_dropout << endl;
    PosteriorDistributionAbstraction res = domain.summary(set2); 
    REQUIRE(res[0].get_lower_bound() == 0); 
    REQUIRE(res[1].get_lower_bound() == 0.6); 
    // cout << res[0].get_lower_bound() << " " << res[0].get_upper_bound() << " " << res[1].get_lower_bound() << " " << res[1].get_upper_bound() << endl;
}
