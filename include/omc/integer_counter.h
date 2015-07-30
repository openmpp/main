/**
 * @file    integer_counter.h
 * Template to produce an iteratable range of integers
 *         
 */
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#pragma once

/**
    * Template to use in range-based for to iterate over a range of integers
    * 
    * Example: for ( int i : integer_counter<-5, 5> ) { }
    * will iterate over the 11 values {-5, -4, ..., 5 }
    *
    * @tparam min_val The minimum value of the range.
    * @tparam max_val The maximum value of the range.
    */

template <typename T, T min_val, T max_val>
class integer_counter {
public:
    // ctor
    integer_counter()
    {
    }

    // embed iterator for simplicity
    class iterator {
        friend class integer_counter;

        // ctor
        iterator(int start)
            : current_val(start)
        { }

    public:
        int operator *() const
        { 
            return current_val;
        }

        const iterator& operator ++()
        {
            ++current_val;
            return *this;
        }

        bool operator !=(const iterator& other) const
        {
            return current_val != other.current_val;
        }

    private:
        T current_val;
    };

    iterator begin() const
    {
        return iterator(min_val);
    }

    iterator end() const
    {
        return iterator(max_val + 1);
    }
};
