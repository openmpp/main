/**
* @file    Range.h
* Declares the Range template
*
*/
// Copyright (c) 2013 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once

#include <array>

using namespace std;


namespace mm {

    template<typename T, T min_val, T max_val>
    class Range
    {
    public:
        // ctors
        Range()
        {
            set(min_val);
            if (!initialization_done) initialize();
        }

        Range(T val)
        {
            set(val);
            if (!initialization_done) initialize();
        }

        // initialize static helper arrays for range-based for iteration
        void initialize()
        {
            for (int i = 0; i < size; ++i) {
                indices[i] = i;
                values[i] = i + min_val;
            }

            initialization_done = true;
        }

        // assignment cover function
        void set(T new_value)
        {
            if (new_value < min_val) {
                value = min_val;
            }
            else if (new_value > max_val) {
                value = max_val;
            }
            else {
                value = new_value;
            }
        }

        // operator: cast to T (use in C++ expression)
        operator T()
        {
            return value;
        }

        // operator: direct assignment
        Range& operator=(T new_value)
        {
            this->set(new_value);
            return *this;
        }

        // operator: assignment by sum
        Range& operator+=(T modify_value)
        {
            T new_value = value + modify_value;
            this->set(new_value);
            return *this;
        }

        // operator: assignment by difference
        Range& operator-=(T modify_value)
        {
            T new_value = value - modify_value;
            this->set(new_value);
            return *this;
        }

        // operator: assignment by product
        Range& operator*=(T modify_value)
        {
            T new_value = value * modify_value;
            this->set(new_value);
            return *this;
        }

        // operator: assignment by quotient
        Range& operator/=(T modify_value)
        {
            T new_value = value / modify_value;
            this->set(new_value);
            return *this;
        }

        // operator: assignment by remainder
        Range& operator%=(T modify_value)
        {
            T new_value = value % modify_value;
            this->set(new_value);
            return *this;
        }

        // operator: assignment by bitwise left shift
        Range& operator<<=(T modify_value)
        {
            T new_value = value << modify_value;
            this->set(new_value);
            return *this;
        }

        // operator: assignment by bitwise right shift
        Range& operator>>=(T modify_value)
        {
            T new_value = value >> modify_value;
            this->set(new_value);
            return *this;
        }

        // operator: assignment by bitwise AND
        Range& operator&=(T modify_value)
        {
            T new_value = value & modify_value;
            this->set(new_value);
            return *this;
        }

        // operator: assignment by bitwise XOR
        Range& operator^=(T modify_value)
        {
            T new_value = value ^ modify_value;
            this->set(new_value);
            return *this;
        }

        // operator: assignment by bitwise OR
        Range& operator|=(T modify_value)
        {
            T new_value = value | modify_value;
            this->set(new_value);
            return *this;
        }

        // operator: prefix increment
        Range& operator++()
        {
            T new_value = value + 1;
            this->set(new_value);
            return *this;
        }

        // operator: prefix decrement
        Range& operator--()
        {
            T new_value = value - 1;
            this->set(new_value);
            return *this;
        }

        // operator: postfix increment
        T operator++(int)
        {
            T new_value = 1 + value;
            this->set(new_value);
            return new_value;
        }

        // operator: postfix decrement
        T operator--(int)
        {
            T new_value = value - 1;
            this->set(new_value);
            return new_value;
        }

        // 0-based value for indexing
        const int pos()
        {
            return (int)value - (int)min_val;
        }

        // methods for range-based for
        Range begin() const { return min; }

        // methods for range-based for
        Range end() const { return max; }

        // storage
        T value;

        // limits (static constants)
        static const T min = min_val;
        static const T max = max_val;
        static const T size = max_val - min_val + 1;

        // Support for range-based for loops in model code
        // Would be more elegant with boost:counting_range or similar.
        static bool initialization_done;
        static array<int, size> values;
        static array<int, size> indices;
    };

    template<typename T, T min_val, T max_val>
    array<int, max_val - min_val + 1> Range<T, min_val, max_val>::indices;

    template<typename T, T min_val, T max_val>
    array<int, max_val - min_val + 1> Range<T, min_val, max_val>::values;

    template<typename T, T min_val, T max_val>
    bool Range<T, min_val, max_val>::initialization_done = false;

} // namespace mm

