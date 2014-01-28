/**
* @file    Range.h
* Declares the Range template
*
*/
// Copyright (c) 2013-2014 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once
#include "omc/integer_counter.h"

using namespace std;

namespace mm {

    /**
    * A range.
    *
    * @tparam T     Storage type for range value.
    * @tparam T_min Minimum value of range.
    * @tparam T_max Maximum value of range.
    */

    template<
        typename T,
        T T_min,
        T T_max
    >
    class Range
    {
    public:
        // ctors
        Range()
            : value(min)
        {}

        Range(T val)
            : value((val < min) ? min : (val > max) ? max : val)
        {}

        // operator: cast to T (use in C++ expression)
        operator T() const
        {
            return value;
        }

        // operator: direct assignment
        Range& operator=(T new_value)
        {
            this->set_value(new_value);
            return *this;
        }

        // operator: assignment by sum
        Range& operator+=(T modify_value)
        {
            int new_value = (int)value + modify_value;
            this->set_value(new_value);
            return *this;
        }

        // operator: assignment by difference
        Range& operator-=(T modify_value)
        {
            int new_value = (int)value - modify_value;
            this->set_value(new_value);
            return *this;
        }

        // operator: assignment by product
        Range& operator*=(T modify_value)
        {
            int new_value = (int)value * modify_value;
            this->set_value(new_value);
            return *this;
        }

        // operator: assignment by quotient
        Range& operator/=(T modify_value)
        {
            int new_value = (int)value / modify_value;
            this->set_value(new_value);
            return *this;
        }

        // operator: assignment by remainder
        Range& operator%=(T modify_value)
        {
            int new_value = (int) value % modify_value;
            this->set_value(new_value);
            return *this;
        }

        // operator: assignment by bitwise left shift
        Range& operator<<=(T modify_value)
        {
            int new_value = (int)value << modify_value;
            this->set_value(new_value);
            return *this;
        }

        // operator: assignment by bitwise right shift
        Range& operator>>=(T modify_value)
        {
            int new_value = (int)value >> modify_value;
            this->set_value(new_value);
            return *this;
        }

        // operator: assignment by bitwise AND
        Range& operator&=(T modify_value)
        {
            int new_value = (int)value & modify_value;
            this->set_value(new_value);
            return *this;
        }

        // operator: assignment by bitwise XOR
        Range& operator^=(T modify_value)
        {
            int new_value = (int)value ^ modify_value;
            this->set_value(new_value);
            return *this;
        }

        // operator: assignment by bitwise OR
        Range& operator|=(T modify_value)
        {
            int new_value = (int)value | modify_value;
            this->set_value(new_value);
            return *this;
        }

        // operator: prefix increment
        Range& operator++()
        {
            int new_value = (int)value + 1;
            this->set_value(new_value);
            return *this;
        }

        // operator: prefix decrement
        Range& operator--()
        {
            int new_value = (int)value - 1;
            this->set_value(new_value);
            return *this;
        }

        // operator: postfix increment
        T operator++(int)
        {
            int new_value = 1 + (int)value;
            return this->set_value(new_value);
        }

        // operator: postfix decrement
        T operator--(int)
        {
            int new_value = (int)value - 1;
            return this->set_value(new_value);
        }

        // 0-based index corresponding to value
        size_t to_index()
        {
            return value - min;
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

        // convert given value to 0-based index
        static size_t to_index(T value)
        {
            return value - min;
        }

        // convert given 0-based index to value
        static T to_value(size_t index)
        {
            return (T)(index + min);
        }

        // limits (static constants)
        static const T min = T_min;
        static const T max = T_max;
        static const size_t size = (int)T_max - (int)T_min + 1;

    private:
        // assignment cover function
        T set_value(int new_value)
        {
            return value = (T)((new_value < min) ? min : (new_value > max) ? max : new_value);
        }

        // storage - the index of the interval in the partition
        T value;
    };

} // namespace mm

