/**
* @file    Partition.h
* Declares the Partition template
*
*/
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#pragma once
#include <map>
#include <array>
#include <cfloat>       // for real
#include "omc/integer_counter.h"
#include "om_types0.h"  // for real

/**
    * A partition.
    *
    * @tparam T     Storage type for partition value (interval, 0-based).  Must fall within limits of int.
    * @tparam T_size Number of intervals in partition
    * @tparam T_lower  Array containing lower limit of each interval in the partition.
    * @tparam T_upper  Array containing upper limit of each interval in the partition.
    * @tparam T_set Set used by find_interval to find the interval for a given number.
    */

template<
    typename T,
    T T_size,
    const std::array<real, T_size> &T_lower,
    const std::array<real, T_size> &T_upper,
    const std::map<real, T> &T_splitter
>
class Partition
{
public:
    typedef T type;

    // ctors
    Partition()
        : interval(0)
    {}

    Partition(T interval)
        : interval((interval < 0 ) ? 0 : (interval > max) ? max : interval)
    {}

    // operator: user-defined conversion
    operator T() const
    {
        return interval;
    }

    // operator: direct assignment (by interval, 0-based)
    Partition& operator=(T new_interval)
    {
        this->set_interval(new_interval);
        return *this;
    }

    // operator: assignment by sum (jump intervals upwards)
    Partition& operator+=(T delta_intervals)
    {
        int new_interval = (int)interval + delta_intervals;
        this->set_interval(new_interval);
        return *this;
    }

    // operator: assignment by difference
    Partition& operator-=(T delta_intervals)
    {
        int new_interval = (int)interval - delta_intervals;
        this->set_interval(new_interval);
        return *this;
    }

    // operator: prefix increment
    Partition& operator++()
    {
        int new_interval = (int)interval + 1;
        this->set_interval(new_interval);
        return *this;
    }

    // operator: prefix decrement
    Partition& operator--()
    {
        int new_interval = (int)interval - 1;
        this->set_interval(new_interval);
        return *this;
    }

    // operator: postfix increment
    T operator++(int)
    {
        int new_interval = 1 + (int)interval;
        return this->set_interval(new_interval);
    }

    // operator: postfix decrement
    T operator--(int)
    {
        int new_interval = (int)interval - 1;
        return this->set_interval(new_interval);
    }

    /**
     * return lower limit of this interval.
     *
     * @return A real.
     */
    real lower() {
        return T_lower[interval];
    }

    /**
     * return upper limit of this interval.
     *
     * @return A real.
     */
    real upper() {
        return T_upper[interval];
    }

    /**
     * return width of this interval.
     * 
     * If the interval is unbounded on the left or the right, return the maximum representable value.
     *
     * @return A T.
     */
    real width() {
        if (interval == 0 && T_lower[0] == -REAL_MAX) {
            return REAL_MAX;
        }
        else if (interval == max && T_upper[max] == REAL_MAX) {
            return REAL_MAX;
        }
        else {
            return T_upper[interval] - T_lower[interval];
        }
    }

    /**
     * Sets the partition of this interval based on a value.
     * 
     * Assigns the interval which contains the argument value.
     *
     * @param val The value.
     *
     * @return A T.
     */
    T set_from_value(real value)
    {
        return set_interval(value_to_interval(value));
    }

    // return an integer_counter object for iterating this partition
    static integer_counter<int, 0, T_size - 1> indices()
    {
        return integer_counter<int, 0, T_size - 1>();
    }

    // return reference to array containing lower value of intervals
    static const std::array<real, T_size> lower_bounds()
    {
        return T_lower;
    }

    // return reference to array containing upper value of intervals
    static const std::array<real, T_size> upper_bounds()
    {
        return T_upper;
    }

    // Find the interval within which a value falls.
    static T value_to_interval(real value)
    {
        // find first interval whose upper bound exceeds value
        auto it = T_splitter.upper_bound(value);
        return (it == T_splitter.end()) ? max : it->second;
    }

    // limits (static constants) - declarations
#if !defined(_MSC_VER)
    // Declared here and defined below in separate template statements
    static const T min;
    static const T max;
    static const int size;
#else // defined(_MSC_VER)
    // MSVC bug workaround 2020-07-03
    // With standard idiom, MSVC does not treat these as compile time constants.
    static const T min = 0;
    static const T max = T_size - 1;
    static const int size = T_size;
#endif //!defined(_MSC_VER)


private:
    // assignment cover function
    T set_interval(int new_interval)
    {
        return interval = (T) ((new_interval < min) ? min : (new_interval > max) ? max : new_interval);
    }

    // storage - the interval in the partition (0-based)
    T interval;
};

// limits (static constants) - definitions

#if !defined(_MSC_VER)
template<
    typename T,
    T T_size,
    const std::array<real, T_size> &T_lower,
    const std::array<real, T_size> &T_upper,
    const std::map<real, T> &T_splitter
>
const T Partition<T, T_size, T_lower, T_upper, T_splitter>::min = 0;

template<
    typename T,
    T T_size,
    const std::array<real, T_size> &T_lower,
    const std::array<real, T_size> &T_upper,
    const std::map<real, T> &T_splitter
>
const T Partition<T, T_size, T_lower, T_upper, T_splitter>::max = T_size - 1;

template<
    typename T,
    T T_size,
    const std::array<real, T_size> &T_lower,
    const std::array<real, T_size> &T_upper,
    const std::map<real, T> &T_splitter
>
const int Partition<T, T_size, T_lower, T_upper, T_splitter>::size = T_size;
#else // defined(_MSC_VER)
    // MSVC bug workaround 2020-07-03
    // With standard idiom, MSVC does not treat these as compile time constants.
#endif // !defined(_MSC_VER)
