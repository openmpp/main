/**
* @file    Partition.h
* Declares the Partition template
*
*/
// Copyright (c) 2013 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once
#include <map>
#include <array>
#include "omc/range_int.h"
#include "om_types0.h" // for real

using namespace std;

namespace mm {


    /**
     * A partition.
     *
     * @tparam T     Storage type for partition value (index).  Must fall within limits of int.
     * @tparam T_size Number of intervals in partition
     * @tparam T_lower  Array containing lower limit of each interval in the partition.
     * @tparam T_upper  Array containing upper limit of each interval in the partition.
     * @tparam T_set Set used by find_interval to find the interval for a given number.
     */

    template<
        typename T,
        T T_size,
        const array<real, T_size> &T_lower,
        const array<real, T_size> &T_upper,
        const map<real, T> &T_splitter
    >
    class Partition
    {
    public:
        // ctors
        Partition()
            : value(0)
        {}

        Partition(T val)
            : value((val < 0 ) ? 0 : (val > max) ? max : val)
        {}

        // operator: cast to T (use in C++ expression)
        operator T() const
        {
            return value;
        }

        // operator: direct assignment
        Partition& operator=(T new_value)
        {
            this->set_value(new_value);
            return *this;
        }

        // operator: assignment by sum
        Partition& operator+=(T modify_value)
        {
            int new_value = (int)value + modify_value;
            this->set_value(new_value);
            return *this;
        }

        // operator: assignment by difference
        Partition& operator-=(T modify_value)
        {
            int new_value = (int)value - modify_value;
            this->set_value(new_value);
            return *this;
        }

        // operator: assignment by product
        Partition& operator*=(T modify_value)
        {
            int new_value = (int)value * modify_value;
            this->set_value(new_value);
            return *this;
        }

        // operator: assignment by quotient
        Partition& operator/=(T modify_value)
        {
            int new_value = (int)value / modify_value;
            this->set_value(new_value);
            return *this;
        }

        // operator: assignment by remainder
        Partition& operator%=(T modify_value)
        {
            int new_value = (int)value % modify_value;
            this->set_value(new_value);
            return *this;
        }

        // operator: assignment by bitwise left shift
        Partition& operator<<=(T modify_value)
        {
            int new_value = (int)value << modify_value;
            this->set_value(new_value);
            return *this;
        }

        // operator: assignment by bitwise right shift
        Partition& operator>>=(T modify_value)
        {
            int new_value = (int)value >> modify_value;
            this->set_value(new_value);
            return *this;
        }

        // operator: assignment by bitwise AND
        Partition& operator&=(T modify_value)
        {
            int new_value = (int)value & modify_value;
            this->set_value(new_value);
            return *this;
        }

        // operator: assignment by bitwise XOR
        Partition& operator^=(T modify_value)
        {
            int new_value = (int)value ^ modify_value;
            this->set_value(new_value);
            return *this;
        }

        // operator: assignment by bitwise OR
        Partition& operator|=(T modify_value)
        {
            int new_value = (int)value | modify_value;
            this->set_value(new_value);
            return *this;
        }

        // operator: prefix increment
        Partition& operator++()
        {
            int new_value = (int)value + 1;
            this->set_value(new_value);
            return *this;
        }

        // operator: prefix decrement
        Partition& operator--()
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

        // return a range_int object for iterating this partition
        static range_int<0, T_size - 1> indices()
        {
            return range_int<0, T_size - 1>();
        }

        // return reference to array containing lower value of intervals
        static const array<real, T_size> lower_bounds()
        {
            return T_lower;
        }

        // return reference to array containing upper value of intervals
        static const array<real, T_size> upper_bounds()
        {
            return T_upper;
        }

        // Find the interval within which a value falls.
        static T to_index(real value)
        {
            // find first interval whose upper bound exceeds value
            auto it = T_splitter.upper_bound(value);
            return (it == T_splitter.end()) ? max : it->second;
        }

        // limits (static constants)
        static const T min = 0;
        static const T max = T_size - 1;
        static const size_t size = T_size;

    private:
        // assignment cover function
        T set_value(int new_value)
        {
            return value = (T) ((new_value < min) ? min : (new_value > max) ? max : new_value);
        }

        // storage - the index of the interval in the partition
        T value;
    };

} // namespace mm

