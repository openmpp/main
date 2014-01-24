/**
* @file    Classification.h
* Declares the Classification template
*
*/
// Copyright (c) 2013 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once
#include "omc/integer_counter.h"

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
            : value(val) // C++ compiler provides value protection through enum
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
            int new_value = (int)value + 1;
            this->set_value(new_value);
            return *this;
        }

        // operator: prefix decrement
        Classification& operator--()
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

        // return a integer_counter object for iterating this classification
        static integer_counter<0, T_size - 1> indices()
        {
            return integer_counter<0, T_size - 1>();
        }

        // limits (static constants)
        static const T min = (T) 0;
        static const T max = (T) (T_size - 1);
        static const size_t size = T_size;

    private:
        // assignment cover function
        T set_value(int new_value)
        {
            return value = (T)((new_value < min) ? min : (new_value > max) ? max : new_value);
        }

        // storage - the level in the classification
        T value;
    };

} // namespace mm

