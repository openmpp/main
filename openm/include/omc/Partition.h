/**
* @file    Partition.h
* Declares the Partition template
*
*/
// Copyright (c) 2013 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once
#include <array>
#include "om_types0.h" // for real


using namespace std;

namespace mm {

    template<typename T, T size_val, real *cut_points_val>
    class Partition
    {
    public:
        // ctors
        Partition()
        {
            set(0);
            if (!initialization_done) initialize();
        }

        Partition(T val)
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
                value = 0;
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
        Partition& operator=(T new_value)
        {
            this->set(new_value);
            return *this;
        }

        // operator: assignment by sum
        Partition& operator+=(T modify_value)
        {
            T new_value = value + modify_value;
            this->set(new_value);
            return *this;
        }

        // operator: assignment by difference
        Partition& operator-=(T modify_value)
        {
            T new_value = value - modify_value;
            this->set(new_value);
            return *this;
        }

        // operator: assignment by product
        Partition& operator*=(T modify_value)
        {
            T new_value = value * modify_value;
            this->set(new_value);
            return *this;
        }

        // operator: assignment by quotient
        Partition& operator/=(T modify_value)
        {
            T new_value = value / modify_value;
            this->set(new_value);
            return *this;
        }

        // operator: assignment by remainder
        Partition& operator%=(T modify_value)
        {
            T new_value = value % modify_value;
            this->set(new_value);
            return *this;
        }

        // operator: assignment by bitwise left shift
        Partition& operator<<=(T modify_value)
        {
            T new_value = value << modify_value;
            this->set(new_value);
            return *this;
        }

        // operator: assignment by bitwise right shift
        Partition& operator>>=(T modify_value)
        {
            T new_value = value >> modify_value;
            this->set(new_value);
            return *this;
        }

        // operator: assignment by bitwise AND
        Partition& operator&=(T modify_value)
        {
            T new_value = value & modify_value;
            this->set(new_value);
            return *this;
        }

        // operator: assignment by bitwise XOR
        Partition& operator^=(T modify_value)
        {
            T new_value = value ^ modify_value;
            this->set(new_value);
            return *this;
        }

        // operator: assignment by bitwise OR
        Partition& operator|=(T modify_value)
        {
            T new_value = value | modify_value;
            this->set(new_value);
            return *this;
        }

        // operator: prefix increment
        Partition& operator++()
        {
            T new_value = value + 1;
            this->set(new_value);
            return *this;
        }

        // operator: prefix decrement
        Partition& operator--()
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

        // test if value falls within Partition limits
        static bool within(int value)
        {
            return (value >= 0) && (value <= max);
        }

        // location of array containing cut-points
        static double *cut_points() { return cut_points_val; }

        // storage
        T value;

        // limits (static constants)
        static const T max = size_val - 1;
        static const T size = size_val;

        static bool initialization_done;

        // Support for range-based for loops in model code
        // Would be more elegant with boost::counting_range or similar.
        static array<int, size> indices;
    };

    template<typename T, T size_val, real *cut_points>
    array<int, size_val> Partition<T, size_val, cut_points>::indices;

    template<typename T, T size_val, real *cut_points>
    bool Partition<T, size_val, cut_points>::initialization_done = false;

} // namespace mm

