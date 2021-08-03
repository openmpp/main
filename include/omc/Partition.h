/**
* @file    Partition.h
* Declares the Partition template
*
*/
// Copyright (c) 2013-2021 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#pragma once
#include <map>
#include <array>
#include <cfloat>       // for real
#include "omc/integer_counter.h"
#include "om_types0.h"  // for real
#include "omc/globals0.h" // for error message handlers

/**
    * A partition.
    *
    * @tparam T     Storage type for partition value (interval, 0-based).  Must fall within limits of int.
    * @tparam T_size Number of intervals in partition
    * @tparam T_lower  Array containing lower limit of each interval in the partition.
    * @tparam T_upper  Array containing upper limit of each interval in the partition.
    * @tparam T_set Set used by find_interval to find the interval for a given number.
    * @tparam NT_name Name of the range (non-type argument).
    */

template<
    typename T,
    T T_size,
    const std::array<real, T_size> &T_lower,
    const std::array<real, T_size> &T_upper,
    const std::map<real, T> &T_splitter,
    std::string const* NT_name
>
class Partition
{
public:
    typedef T type;

    // ctors
    Partition()
        : interval(0)
    {}

    Partition(int val)
    {
        set_interval(val);
    }

    // operator: implicit conversion to int
    operator int() const
    {
        return get();
    }

    // operator: direct assignment (by interval, 0-based)
    Partition& operator=(int new_interval)
    {
        this->set_interval(new_interval);
        return *this;
    }

    // operator: assignment by sum (jump intervals upwards)
    Partition& operator+=(int delta_intervals)
    {
        int new_interval = get() + delta_intervals;
        this->set_interval(new_interval);
        return *this;
    }

    // operator: assignment by difference
    Partition& operator-=(int delta_intervals)
    {
        int new_interval = get() - delta_intervals;
        this->set_interval(new_interval);
        return *this;
    }

    // operator: prefix increment
    Partition& operator++()
    {
        int new_interval = get() + 1;
        this->set_interval(new_interval);
        return *this;
    }

    // operator: prefix decrement
    Partition& operator--()
    {
        int new_interval = get() - 1;
        this->set_interval(new_interval);
        return *this;
    }

    // operator: postfix increment
    int operator++(int)
    {
        int new_interval = 1 + get();
        return this->set_interval(new_interval);
    }

    // operator: postfix decrement
    int operator--(int)
    {
        int new_interval = get() - 1;
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
    int set_from_value(real value)
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
    static int value_to_interval(real value)
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

    /**
     * Gets the name of the range
     *
     * @return The name.
     */
    static const std::string& get_name()
    {
        return *NT_name;
    }

private:
    // cover function to get value
    int get() const
    {
        return interval;
    }

    // assignment cover function
    int set_interval(int new_interval)
    {
        int trunced_interval = ((new_interval < min) ? min : (new_interval > max) ? max : new_interval);
        if (om_bounds_errors) { // is constexpr
            if (new_interval != trunced_interval) {
                handle_bounds_error("partition " + get_name(), min, max, new_interval);
            }
        }

        return interval = (T)trunced_interval;
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
    const std::map<real, T> &T_splitter,
    std::string const* NT_name
>
const T Partition<T, T_size, T_lower, T_upper, T_splitter, NT_name>::min = 0;

template<
    typename T,
    T T_size,
    const std::array<real, T_size> &T_lower,
    const std::array<real, T_size> &T_upper,
    const std::map<real, T> &T_splitter,
    std::string const* NT_name
>
const T Partition<T, T_size, T_lower, T_upper, T_splitter, NT_name>::max = T_size - 1;

template<
    typename T,
    T T_size,
    const std::array<real, T_size> &T_lower,
    const std::array<real, T_size> &T_upper,
    const std::map<real, T> &T_splitter,
    std::string const* NT_name
>
const int Partition<T, T_size, T_lower, T_upper, T_splitter, NT_name>::size = T_size;
#else // defined(_MSC_VER)
    // MSVC bug workaround 2020-07-03
    // With standard idiom, MSVC does not treat these as compile time constants.
#endif // !defined(_MSC_VER)
