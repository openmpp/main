/**
* @file    Classification.h
* Declares the Classification template
*
*/
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#pragma once
#include "omc/integer_counter.h"

using namespace std;

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
    typedef T type;

    /**
     * Default ctor
     */
    Classification()
        : enum_value((T)0)
    {}

    /**
     * Converting ctor from enum
     *
     * @param val The value.
     */
    Classification(T val)
        : enum_value(val)
    {}

    /**
     * Converting ctor from int
     * 
     * Not preferred, since type checking is bypassed.  For compatibility
     * with older code.  Value is trunced to valid range of underlying enum.
     *
     * @param val The value.
     */
    Classification(int val)
    {
        set_value(val);
    }

    // operator: user-defined conversion to enum
    operator T() const
    {
        return enum_value;
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
        int new_value = get() + 1;
        this->set_value(new_value);
        return *this;
    }

    // operator: prefix decrement
    Classification& operator--()
    {
        int new_value = get() - 1;
        this->set_value(new_value);
        return *this;
    }

    // operator: postfix increment
    T operator++(int)
    {
        int new_value = 1 + get();
        return this->set_value(new_value);
    }

    // operator: postfix decrement
    T operator--(int)
    {
        int new_value = get() - 1;
        return this->set_value(new_value);
    }

    // return a integer_counter object for iterating this classification
    static integer_counter<int, 0, T_size - 1> indices()
    {
        return integer_counter<int, 0, T_size - 1>();
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
    static const T min = (T) 0;
    static const T max = (T) (T_size - 1);
    static const int size = T_size;
#endif //!defined(_MSC_VER)

private:
    // cover function to get value
    int get() const
    {
        return (int)enum_value;
    }

    // assignment cover function
    T set_value(int new_value)
    {
        int trunced_value = ((new_value < min) ? min : (new_value > max) ? max : new_value);
        return enum_value = (T)trunced_value;
    }

    // storage - the level in the classification
    T enum_value;
};

// limits (static constants) - definitions

#if !defined(_MSC_VER)
template<
    typename T,
    size_t T_size
>
const T Classification<T, T_size>::min = (T) 0;

template<
    typename T,
    size_t T_size
>
const T Classification<T, T_size>::max = (T) (T_size - 1);

template<
    typename T,
    size_t T_size
>
const int Classification<T, T_size>::size = T_size;
#else // defined(_MSC_VER)
    // MSVC bug workaround 2020-07-03
    // With standard idiom, MSVC does not treat these as compile time constants.
#endif // !defined(_MSC_VER)
