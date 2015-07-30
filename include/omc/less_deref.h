/**
 * @file    less_deref.h
 * Declares the less_deref functor
 *         
 */
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#pragma once

/**
    * Functor for operator< on dereferenced pointers
    * For use as comparison function in std::set which stores pointers.
    *
    * @tparam  T   Generic type parameter.
    */

template<class T>
struct less_deref
{	
    bool operator()(const T& lhs, const T& rhs) const
    {
        return ( *lhs < *rhs );
    }
};

