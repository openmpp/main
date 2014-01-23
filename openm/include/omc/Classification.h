/**
* @file    Classification.h
* Declares the Classification template
*
*/
// Copyright (c) 2013 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once
#include <array>

using namespace std;

namespace mm {

    template<typename T, int size_val>
    class Classification
    {
    public:
        // ctors
        Classification()
        {
            set(0);
            if (!initialization_done) initialize();
        }

        Classification(T val)
        {
            set(val);
            if (!initialization_done) initialize();
        }

        void initialize()
        {
            // initialize static helper arrays for range-based for in model code
            for (int i = 0; i < size; ++i) {
                indices[i] = i;
            }
            initialization_done = true;
        }

        // assignment cover function
        void set(T new_value)
        {
            if (new_value < 0) {
                value = (T)0;
            }
            else if (new_value > max) {
                value = (T)max;
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
        Classification& operator=(T new_value)
        {
            this->set(new_value);
            return *this;
        }

        // operator: prefix increment
        Classification& operator++()
        {
            T new_value = value + 1;
            this->set(new_value);
            return *this;
        }

        // operator: prefix decrement
        Classification& operator--()
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

        // test if value falls within Classification limits
        static bool within(int value)
        {
            return (value >= 0) && (value <= max);
        }

        // storage - the index of the interval in the partition
        T value;

        // limits (static constants)
        static const int max = size_val - 1;
        static const int size = size_val;

        static bool initialization_done;

        // Support for range-based for loops in model code
        // Would be more elegant with boost::counting_range or similar.
        static array<int, size> indices;
    };

    template<typename T, int size_val>
    bool Classification<T, size_val>::initialization_done = false;

    template<typename T, int size_val>
    array<int, size_val> Classification<T, size_val>::indices;

} // namespace mm

