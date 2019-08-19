#ifndef INTERVAL_H
#define INTERVAL_H

#include <algorithm> // For std::min_element (and max)
#include <string>
// Cannot 'using namespace std' since there is some namespace collision
// with the operator+ overload.


// Template for closed intervals.
// We assume T is a standard numeric type;
// specifically, std::to_string and arithmetic are defined,
// totally ordered, 0 and 1 are elements, ...
template <typename T>
class Interval {
private:
    T lower_bound;
    T upper_bound;

    bool empty_flag;

public:
    Interval();
    Interval(const T &single_value);
    Interval(const T &lower_bound, const T &upper_bound);
    Interval(const Interval<T> &other);

    T get_lower_bound() const { return lower_bound; }
    T get_upper_bound() const { return upper_bound; }

    bool isEmpty() const { return empty_flag; }

    Interval<T> operator- () const;
    Interval<T> operator+ (const Interval<T> &right) const;
    Interval<T> operator- (const Interval<T> &right) const;
    Interval<T> operator* (const Interval<T> &right) const;
    Interval<T> operator/ (const Interval<T> &right) const;

    // Equality is checking if the intervals express the same set,
    // not if equality holds on all concretizations
    bool operator== (const Interval<T> &right) const;

    static Interval<T> join(const Interval<T> &e1, const Interval<T> &e2);
};


template <typename T>
Interval<T>::Interval() {
    empty_flag = true;
}

template <typename T>
Interval<T>::Interval(const T &single_value) {
    lower_bound = single_value;
    upper_bound = single_value;
    empty_flag = false;
}

template <typename T>
Interval<T>::Interval(const T &lower_bound, const T &upper_bound) {
    this->lower_bound = lower_bound;
    this->upper_bound = upper_bound;
    empty_flag = false;
}

template <typename T>
Interval<T>::Interval(const Interval<T> &other) {
    lower_bound = other.lower_bound;
    upper_bound = other.upper_bound;
    empty_flag = other.empty_flag;
}

template <typename T>
std::string to_string(const Interval<T> &interval) {
    return "[" + std::to_string((T)interval.get_lower_bound()) + ", " +
                 std::to_string((T)interval.get_upper_bound()) + "]";
}

/**
 * Operators below
 */

template <typename T>
Interval<T> Interval<T>::operator- () const {
    return Interval<T>(-this->upper_bound, -this->lower_bound);
}

template <typename T>
Interval<T> Interval<T>::operator+ (const Interval<T> &right) const {
    if(this->isEmpty() || right.isEmpty()) {
        // Return an empty interval
        return Interval<T>();
    }
    return Interval<T>(this->lower_bound + right.lower_bound,
                       this->upper_bound + right.upper_bound);
}

template <typename T>
Interval<T> Interval<T>::operator- (const Interval<T> &right) const {
    return *this + (-right);
}

template <typename T>
Interval<T> Interval<T>::operator* (const Interval<T> &right) const {
    if(this->isEmpty() || right.isEmpty()) {
        // Return an empty interval
        return Interval<T>();
    }
    T points[4] = {this->lower_bound * right.lower_bound,
                   this->lower_bound * right.upper_bound,
                   this->upper_bound * right.lower_bound,
                   this->upper_bound * right.upper_bound};
    T lower_bound = *std::min_element(points, points+4);
    T upper_bound = *std::max_element(points, points+4);
    return Interval<T>(lower_bound, upper_bound);
}

template <typename T>
Interval<T> Interval<T>::operator/ (const Interval<T> &right) const {
    if(this->isEmpty() || right.isEmpty()) {
        // Return an empty interval
        return Interval<T>();
    }
    // XXX incorrect if division by zero is possible
    Interval<T> right_reciprocal((T)1/right.upper_bound, (T)1/right.lower_bound);
    return *this * right_reciprocal;
}

template <typename T>
bool Interval<T>::operator== (const Interval<T> &right) const {
    if(this->isEmpty() && right.isEmpty()) {
        return true;
    } else if(!this->isEmpty() && !right.isEmpty()) {
        return this->lower_bound == right.lower_bound && this->upper_bound == right.upper_bound;
    } else {
        return false;
    }
}

template <typename T>
Interval<T> Interval<T>::join(const Interval<T> &e1, const Interval<T> &e2) {
    if(e1.isEmpty()) {
        return e2;
    } else if(e2.isEmpty()) {
        return e1;
    }
    // Imprecise join if no overlap
    T lower_bound = e1.lower_bound < e2.lower_bound ? e1.lower_bound : e2.lower_bound;
    T upper_bound = e1.upper_bound > e2.upper_bound ? e1.upper_bound : e2.upper_bound;
    return Interval<T>(lower_bound, upper_bound);
}

#endif
