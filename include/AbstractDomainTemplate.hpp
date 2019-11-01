#ifndef ABSTRACTDOMAINTEMPLATE_HPP
#define ABSTRACTDOMAINTEMPLATE_HPP

#include <vector>


/**
 * The following two classes form a very generic framework for abstract domains.
 * Subclassing AbstractElement gives you the representation of the abstraction,
 * and subclassing AbstractDomain (using the AbstractElement subclass as the template parameter)
 * gives you the abstract domain object.
 */

// TODO should we include a template <typename C> here for the concrete type?
class AbstractElement {
public:
    virtual bool isBottomElement() const = 0;
    // TODO include more logic for common things with the bottom element flag / check
};


template <typename A>
class AbstractDomainTemplate {
public:
    // We must give them different names; overriding one in a subclass
    // causes the compiler to skip over any functions in the same name as the base class
    virtual A binary_join(const A &e1, const A &e2) const = 0;
    A join(const std::vector<A> &elements) const; 
};


/**
 * In creating the abstract class,
 * we must leave at least one of binary_join(e1, e2) or join([es]) as pure virtual.
 * We can provide a default implementation for either
 * binary_join(e1, e2) in terms of join([es]):
 *      template <typename A>
 *      A AbstractDomain<A>::binary_join(const A &e1, const A &e2) const {
 *          return join({e1, e2});
 *      }
 * or vice-versa (which is more complicated and thus left uncommented):
 */
template <typename A>
A AbstractDomainTemplate<A>::join(const std::vector<A> &elements) const {
    A ret;
    for(auto i = elements.cbegin(); i != elements.cend(); i++) {
        ret = binary_join(ret, *i);
    }
    return ret;
}

#endif
