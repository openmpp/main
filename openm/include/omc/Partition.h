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
     * @tparam T     Storage type for partition value (index).
     * @tparam T_min Minimum value of partition value (always 0 for partitions).
     * @tparam T_max Maximum value of partition value (index).
     * @tparam T_lower  Array containing lower limit of each interval in the partition.
     * @tparam T_upper  Array containing upper limit of each interval in the partition.
     * @tparam T_set Set used by find_interval to find the interval for a given number.
     */

    template<
        typename T,
        T T_min,
        T T_max,
        const array<real, T_max - T_min + 1> &T_lower,
        const array<real, T_max - T_min + 1> &T_upper,
        const map<real, T> &T_splitter>
    class Partition
    {
    public:
        // ctors
        Partition()
            : value(T_min)
        {}

        Partition(T val)
            : value((val < T_min ) ? T_min : (val > T_max) ? T_max : val)
        {}

        // assignment cover function
        void set_value(T new_value)
        {
            value = (new_value < T_min) ? T_min : (new_value > T_max) ? T_max : new_value;
        }

        // operator: cast to T (use in C++ expression)
        operator T()
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
            T new_value = value + modify_value;
            this->set_value(new_value);
            return *this;
        }

        // operator: assignment by difference
        Partition& operator-=(T modify_value)
        {
            T new_value = value - modify_value;
            this->set_value(new_value);
            return *this;
        }

        // operator: assignment by product
        Partition& operator*=(T modify_value)
        {
            T new_value = value * modify_value;
            this->set_value(new_value);
            return *this;
        }

        // operator: assignment by quotient
        Partition& operator/=(T modify_value)
        {
            T new_value = value / modify_value;
            this->set_value(new_value);
            return *this;
        }

        // operator: assignment by remainder
        Partition& operator%=(T modify_value)
        {
            T new_value = value % modify_value;
            this->set_value(new_value);
            return *this;
        }

        // operator: assignment by bitwise left shift
        Partition& operator<<=(T modify_value)
        {
            T new_value = value << modify_value;
            this->set_value(new_value);
            return *this;
        }

        // operator: assignment by bitwise right shift
        Partition& operator>>=(T modify_value)
        {
            T new_value = value >> modify_value;
            this->set_value(new_value);
            return *this;
        }

        // operator: assignment by bitwise AND
        Partition& operator&=(T modify_value)
        {
            T new_value = value & modify_value;
            this->set_value(new_value);
            return *this;
        }

        // operator: assignment by bitwise XOR
        Partition& operator^=(T modify_value)
        {
            T new_value = value ^ modify_value;
            this->set_value(new_value);
            return *this;
        }

        // operator: assignment by bitwise OR
        Partition& operator|=(T modify_value)
        {
            T new_value = value | modify_value;
            this->set_value(new_value);
            return *this;
        }

        // operator: prefix increment
        Partition& operator++()
        {
            T new_value = value + 1;
            this->set_value(new_value);
            return *this;
        }

        // operator: prefix decrement
        Partition& operator--()
        {
            T new_value = value - 1;
            this->set_value(new_value);
            return *this;
        }

        // operator: postfix increment
        T operator++(int)
        {
            T new_value = 1 + value;
            this->set_value(new_value);
            return new_value;
        }

        // operator: postfix decrement
        T operator--(int)
        {
            T new_value = value - 1;
            this->set_value(new_value);
            return new_value;
        }

        // return a range_int object for iterating this partition
        static range_int<T_min, T_max> indices()
        {
            return range_int<T_min, T_max>();
        }

        // test if value (index) falls within limits
        static bool within(int value)
        {
            return (value >= T_min) && (value <= T_max);
        }

        // return reference to array containing lower value of intervals
        static const array<real, T_max - T_min + 1> lower_bound()
        {
            return T_lower;
        }

        // return reference to array containing upper value of intervals
        static const array<real, T_max - T_min + 1> upper_bound()
        {
            return T_upper;
        }

        // Find the interval within which a value falls.
        static T to_index(real value)
        {
            // find first interval whose upper bound exceeds value
            auto it = T_splitter.upper_bound(value);
            return (it == T_splitter.end()) ? T_max : it->second;
        }

        // storage - the index of the interval in the partition
        T value;

        // limits (static constants)
        static const T min = T_min;
        static const T max = T_max;
        static const size_t size = (size_t)T_max - (size_t)T_min + 1;
    };

} // namespace mm

