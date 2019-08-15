#ifndef INTERVAL_H
#define INTERVAL_H

#include <algorithm> // For std::min_element (and max)
#include <string>
// Cannot 'using namespace std' since there is some namespace collision
// with the operator+ overload.

// We "must" forward declare templated friend functions
// (and, in turn, the template class)
// so that, e.g., operator-(Interval<int>) is not a friend to Interval<double>
template <typename T>
class Interval;

template <typename T>
Interval<T> operator- (const Interval<T> &interval);

template <typename T>
Interval<T> operator+ (const Interval<T> &left, const Interval<T> &right);

template <typename T>
Interval<T> operator- (const Interval<T> &left, const Interval<T> &right);

template <typename T>
Interval<T> operator* (const Interval<T> &left, const Interval<T> &right);

template <typename T>
Interval<T> operator/ (const Interval<T> &left, const Interval<T> &right);

template <typename T>
bool operator== (const Interval<T> &left, const Interval<T> &right);


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

    friend Interval<T> operator- <T> (const Interval<T> &interval);
    friend Interval<T> operator+ <T> (const Interval<T> &left, const Interval<T> &right);
    friend Interval<T> operator- <T> (const Interval<T> &left, const Interval<T> &right);
    friend Interval<T> operator* <T> (const Interval<T> &left, const Interval<T> &right);
    friend Interval<T> operator/ <T> (const Interval<T> &left, const Interval<T> &right);

    // Equality is checking if the intervals express the same set,
    // not if equality holds on all concretizations
    friend bool operator== <T> (const Interval<T> &left, const Interval<T> &right);
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
 * Friend functions below
 */

template <typename T>
Interval<T> operator- (const Interval<T> &interval) {
    return Interval<T>(-interval.upper_bound, -interval.lower_bound);
}

template <typename T>
Interval<T> operator+ (const Interval<T> &left, const Interval<T> &right) {
    if(left.isEmpty() || right.isEmpty()) {
        // Return an empty interval
        return left;
    }
    return Interval<T>(left.lower_bound + right.lower_bound,
                       left.upper_bound + right.upper_bound);
}

template <typename T>
Interval<T> operator- (const Interval<T> &left, const Interval<T> &right) {
    return left + (-right);
}

template <typename T>
Interval<T> operator* (const Interval<T> &left, const Interval<T> &right) {
    if(left.isEmpty() || right.isEmpty()) {
        // Return an empty interval
        return left;
    }
    T points[4] = {left.lower_bound * right.lower_bound,
                   left.lower_bound * right.upper_bound,
                   left.upper_bound * right.lower_bound,
                   left.upper_bound * right.upper_bound};
    T lower_bound = *std::min_element(points, points+4);
    T upper_bound = *std::max_element(points, points+4);
    return Interval<T>(lower_bound, upper_bound);
}

template <typename T>
Interval<T> operator/ (const Interval<T> &left, const Interval<T> &right) {
    if(left.isEmpty() || right.isEmpty()) {
        // Return an empty interval
        return left;
    }
    // XXX incorrect if division by zero is possible
    Interval<T> right_reciprocal((T)1/right.upper_bound, (T)1/right.lower_bound);
    return left * right_reciprocal;
}

template <typename T>
bool operator== (const Interval<T> &left, const Interval<T> &right) {
    if(left.isEmpty() && right.isEmpty()) {
        return true;
    } else if(!left.isEmpty() && !right.isEmpty()) {
        return left.lower_bound == right.lower_bound &&
               left.upper_bound == right.upper_bound;
    } else {
        return false;
    }
}

#endif
