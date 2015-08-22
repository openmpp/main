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
     * Default constructor.
     */
    Classification()
        : enum_value((T)0)
    {}

    /**
     * Constructor from enum
     *
     * @param val The value.
     */
    Classification(T val)
        : enum_value(val)
    {}

    /**
     * Constructor from integer
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

    // operator: conversion to T
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

    // limits (static constants)
    static const T min = (T) 0;
    static const T max = (T) (T_size - 1);
    static const int size = T_size;

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


