#ifndef CONCRETECOMMON_H
#define CONCRETECOMMON_H


// You can control the return type of evaluate(X) to support 3-valued logic
template <typename X, typename L=bool>
class Predicate {
private:
public:
    virtual const L evaluate(const X &input) const = 0;
};


// This class is the interface needed for the Concrete Semantics
// P is the predicate type
// PS is the predicate set type,
// D is the type for the posterior distribution
template <typename P, typename PS, typename D>
class DataSetInterface {
private:
public:
    virtual bool isPure() = 0;
    virtual void filter(const P &phi, bool mode) = 0;
    virtual D summary() = 0;
    virtual P* bestSplit(PS *predicates) = 0;
};


#endif
