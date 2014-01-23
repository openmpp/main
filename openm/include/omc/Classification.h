/**
* @file    Classification.h
* Declares the Classification template
*
*/
// Copyright (c) 2013 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once
#include "omc/range_int.h"

using namespace std;

namespace mm {

    /**
    * A partition.
    *
    * @tparam T     Enum of this classification.
    * @tparam T_size Number of levels in this classification.
    */

    template<
        typename T,
        size_t T_size
    >
    class Classification
    {
    public:
        // ctors
        Classification()
            : value((T)0)
        {}

        Classification(T val)
            : value(val)
        {}

        // operator: cast to T (use in C++ expression)
        operator T() const
        {
            return value;
        }

        // operator: direct assignment
        Classification& operator=(T new_value)
        {
            this->set_value(new_value);
            return *this;
        }

        // operator: prefix increment
        Classification& operator++()
        {
            T new_value = (T) (value + 1);
            this->set_value(new_value);
            return *this;
        }

        // operator: prefix decrement
        Classification& operator--()
        {
            T new_value = (T) (value - 1);
            this->set_value(new_value);
            return *this;
        }

        // operator: postfix increment
        T operator++(int)
        {
            T new_value = (T) (1 + value);
            return this->set_value(new_value);
        }

        // operator: postfix decrement
        T operator--(int)
        {
            T new_value = (T) (value - 1);
            return this->set_value(new_value);
        }

        // return a range_int object for iterating this classification
        static range_int<0, T_size - 1> indices()
        {
            return range_int<0, T_size - 1>();
        }

        // limits (static constants)
        static const T min = (T) 0;
        static const T max = (T) (T_size - 1);
        static const size_t size = T_size;

    private:
        // assignment cover function
        T set_value(T new_value)
        {
            return value = (T)((new_value < 0) ? 0 : (new_value >= T_size) ? T_size - 1 : new_value);
        }

        // storage - the level in the classification
        T value;
    };

} // namespace mm

