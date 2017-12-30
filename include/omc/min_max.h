/**
* @file    min_max.h
* Implementation of min, max, clamp templates
*/
// Copyright (c) 2013-2016 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#pragma once

#include <type_traits>

// undefine min and max macros if defined (enables use of this file in Modgen models)
#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif

// Replacement versions in global namespace of std:min and std::max
// which enable type conversion of arguments to a common shared type.
// Specializations of common_type<> for ompp types which enable 
// their particpation are found in their respective header files.

template <typename T, typename U, typename CT = typename std::common_type<T, U>::type>
inline CT min(T a, U b)
{
    // explicit casts used to support user-defined types
    return (CT)a < (CT)b ? (CT)a : (CT)b;
}

template <typename T, typename U, typename CT = typename std::common_type<T, U>::type>
inline CT max(T a, U b)
{
    // explicit casts used to support user-defined types
    return (CT)a < (CT)b ? (CT)b : (CT)a;
}

// Replacement versions in global namespace of std:min and std::max
// which enable type conversion of arguments to a common shared type.
template <typename T, typename U, typename V, typename CT = typename std::common_type<T, U, V>::type>
inline CT clamp(T inf, U sup, V x)
{
    // explicit casts used to support user-defined types
    if ((CT)inf >(CT)x) return (CT)inf;
    else if ((CT)sup < (CT)x) return (CT)sup;
    else return (CT)x;
}
