/**
* @file    Range.h
* Declares the Range template
*
*/
// Copyright (c) 2013-2021 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#pragma once
#include "omc/integer_counter.h"
#include "omc/globals0.h" // for error message handlers

/**
* A range.
*
* @tparam T     Storage type for range value.
* @tparam T_min Minimum value of range.
* @tparam T_max Maximum value of range.
* @tparam NT_name Name of the range (non-type argument).
*/

template<
    typename T,
    int T_min,
    int T_max,
    std::string const* NT_name
>
class Range
{
public:
    typedef T type;

    // ctors
    Range()
        : range_value(min)
    {}

    Range(int val)
    {
        set_value(val);
    }

    // operator: implicit conversion to int
    operator int() const
    {
        return get();
    }

    // operator: direct assignment
    Range& operator=(int new_value)
    {
        this->set_value(new_value);
        return *this;
    }

    // operator: assignment by sum
    Range& operator+=(int modify_value)
    {
        int new_value = get() + modify_value;
        this->set_value(new_value);
        return *this;
    }

    // operator: assignment by difference
    Range& operator-=(int modify_value)
    {
        int new_value = get() - modify_value;
        this->set_value(new_value);
        return *this;
    }

    // operator: assignment by product
    Range& operator*=(int modify_value)
    {
        int new_value = get() * modify_value;
        this->set_value(new_value);
        return *this;
    }

    // operator: assignment by quotient
    Range& operator/=(int modify_value)
    {
        int new_value = get() / modify_value;
        this->set_value(new_value);
        return *this;
    }

    // operator: assignment by remainder
    Range& operator%=(int modify_value)
    {
        int new_value = get() % modify_value;
        this->set_value(new_value);
        return *this;
    }

    // operator: assignment by bitwise left shift
    Range& operator<<=(int modify_value)
    {
        int new_value = get() << modify_value;
        this->set_value(new_value);
        return *this;
    }

    // operator: assignment by bitwise right shift
    Range& operator>>=(int modify_value)
    {
        int new_value = get() >> modify_value;
        this->set_value(new_value);
        return *this;
    }

    // operator: assignment by bitwise AND
    Range& operator&=(int modify_value)
    {
        int new_value = get() & modify_value;
        this->set_value(new_value);
        return *this;
    }

    // operator: assignment by bitwise XOR
    Range& operator^=(int modify_value)
    {
        int new_value = get() ^ modify_value;
        this->set_value(new_value);
        return *this;
    }

    // operator: assignment by bitwise OR
    Range& operator|=(int modify_value)
    {
        int new_value = get() | modify_value;
        this->set_value(new_value);
        return *this;
    }

    // operator: prefix increment
    Range& operator++()
    {
        int new_value = get() + 1;
        this->set_value(new_value);
        return *this;
    }

    // operator: prefix decrement
    Range& operator--()
    {
        int new_value = get() - 1;
        this->set_value(new_value);
        return *this;
    }

    // operator: postfix increment
    int operator++(int)
    {
        int new_value = 1 + get();
        return this->set_value(new_value);
    }

    // operator: postfix decrement
    int operator--(int)
    {
        int new_value = get() - 1;
        return this->set_value(new_value);
    }

    // 0-based index corresponding to value
    int to_index()
    {
        return get() - min;
    }

    // test if value falls within range limits
    static bool within(int value)
    {
        return (value >= min) && (value <= max);
    }

    // test if value falls within range limits
    static bool within(double value)
    {
        return (value >= min) && (value <= max);
    }

    // test if value falls within range limits
    static bool within(long long value)
    {
        return (value >= min) && (value <= max);
    }

    // clamp a quantity to the limits of a range, with 'int' argument returning 'int' result
    static int clamp(int val)
    {
        if (val < min) return min;
        if (val > max) return max;
        else return val;
    }

    // clamp a quantity to the limits of a range, with 'double' argument returning 'double' result
    static double clamp(double val)
    {
        if (val < min) return min;
        if (val > max) return max;
        else return val;
    }

    // clamp a quantity to the limits of a range, with 'float' argument returning 'float' result
    static float clamp(float val)
    {
        if (val < min) return min;
        if (val > max) return max;
        else return val;
    }

    // clamp a quantity to the limits of a range, with 'long long' argument returning 'long long' result
    static long long clamp(long long val)
    {
        // TODO - throw exception if limit of int exceeded?
        if (val < min) return min;
        if (val > max) return max;
        else return val;
    }

    // return a integer_counter object for iterating indices of this range
    static integer_counter<int, 0, T_max - T_min> indices()
    {
        return integer_counter<int, 0, T_max - T_min>();
    }

    // return a integer_counter object for iterating values of this range
    static integer_counter<int, T_min, T_max> values()
    {
        return integer_counter<int, T_min, T_max>();
    }

    // convert a range value to 0-based index
    static int to_index(int val)
    {
        if (val < min) return 0;
        else if (val > max) return max - min;
        else return val - min;
    }

    // convert given 0-based index to value
    static int to_value(int index)
    {
        return (int)(index + min);
    }

    // limits (static constants) - declarations
#if !defined(_MSC_VER)
    // Declared here and defined below in separate template statements
    static const int min;
    static const int max;
    static const int size;
#else // defined(_MSC_VER)
    // MSVC bug workaround 2020-07-03
    // With standard idiom, MSVC does not treat these as compile time constants.
    static const int min = T_min;
    static const int max = T_max;
    static const int size = (int)T_max - (int)T_min + 1;
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
        return range_value;
    }

    // assignment cover function
    int set_value(int new_value)
    {
        int trunced_value = ((new_value < min) ? min : (new_value > max) ? max : new_value);
        if (om_bounds_errors) { // is constexpr
            if (new_value != trunced_value) {
                handle_bounds_error("range " + get_name(), min, max, new_value);
            }
        }
        range_value = (T)trunced_value;
        return trunced_value;
    }

    // storage - the value within the range
    T range_value;
};

// limits (static constants) - definitions

#if !defined(_MSC_VER)
template<
    typename T,
    int T_min,
    int T_max,
    std::string const* NT_name
>
const int Range<T, T_min, T_max, NT_name>::min = T_min;

template<
    typename T,
    int T_min,
    int T_max,
    std::string const* NT_name
>
const int Range<T, T_min, T_max, NT_name>::max = T_max;

template<
    typename T,
    int T_min,
    int T_max,
    std::string const* NT_name
>
const int Range<T, T_min, T_max, NT_name>::size = (int)T_max - (int)T_min + 1;
#else // defined(_MSC_VER)
    // MSVC bug workaround 2020-07-03
    // With standard idiom, MSVC does not treat these as compile time constants.
#endif // !defined(_MSC_VER)
