#ifndef ABSTRACTDOMAIN_H
#define ABSTRACTDOMAIN_H

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
};


template <typename A>
class AbstractDomain {
public:
    virtual A join(const A &e1, const A &e2) const = 0;
    A join(const std::vector<A> &elements) const; 
};


/**
 * In creating the abstract class,
 * we must leave at least one of join(e1, e2) or join([es]) as pure virtual.
 * We can provide a default implementation for either
 * join(e1, e2) in terms of join([es]):
 *      template <typename A>
 *      A AbstractDomain<A>::join(const A &e1, const A &e2) const {
 *          return join({e1, e2});
 *      }
 * or vice-versa (which is more complicated and thus left uncommented):
 */
template <typename A>
A AbstractDomain<A>::join(const std::vector<A> &elements) const {
    A ret;
    for(typename std::vector<A>::const_iterator i = elements.begin(); i != elements.end; i++) {
        ret = join(ret, *i);
    }
    return ret;
}

#endif
