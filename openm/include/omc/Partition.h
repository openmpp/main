/**
* @file    Partition.h
* Declares the Partition template
*
*/
// Copyright (c) 2013 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once
#include <set>
#include <array>
#include "omc/range_int.h"
#include "om_types0.h" // for real

using namespace std;

namespace mm {

    template<typename T, T min_val, T max_val, const set<real> &splitter>
    class Partition
    {
    public:
        // ctors
        Partition()
            : value(min_val)
        {}

        Partition(T val)
            : value((val < min ) ? min : (val > max) ? max : val)
        {}

        // assignment cover function
        void set_value(T new_value)
        {
            value = (val < min) ? min : (val > max) ? max : val;
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

        // test if value (index) falls within limits
        static bool within(int value)
        {
            return (value >= min) && (value <= max);
        }

        // Determine interval within which a value falls.
        static T find_interval(real value)
        {
            auto it = splitter.lower_bound(value);
            return (it == end()) ? max : *it;
        }

        // storage - the index of the interval in the partition
        T value;

        // limits (static constants)
        static const T min = min_val;
        static const T max = max_val;
        static const size_t size = max - min + 1;

        // Support for range-based for loops in model code
        static range_int<min, max> indices;

        // Splitter set for fast determination of interval

    };

    template<typename T, T min, T max, const set<real> &splitter>
    range_int<min, max> Partition<T, min, max, splitter>::indices;

} // namespace mm

