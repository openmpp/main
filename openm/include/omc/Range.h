/**
* @file    Range.h
* Declares the Range template
*
*/
// Copyright (c) 2013 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once

using namespace std;

namespace mm {

template<typename T, T min_arg, T max_arg>
class Range
{
public:
    // ctors
    Range()
    {
        set(min_arg);
    }

    Range(T val)
    {
        set(val);
    }

    // assignment cover function
    void set(T new_value)
    {
        if (new_value < min) {
            value = min;
        }
        else if (new_value > max) {
            value = max;
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

    // storage
    T value;

    // limits (static constants)
    static const T min;
    static const T max;
    static const T size;
};

template<typename T, T min_arg, T max_arg>
const T Range<T, min_arg, max_arg>::min = min_arg;

template<typename T, T min_arg, T max_arg>
const T Range<T, min_arg, max_arg>::max = max_arg;

template<typename T, T min_arg, T max_arg>
const T Range<T, min_arg, max_arg>::size = max_arg - min_arg + 1;


} // namespace mm
