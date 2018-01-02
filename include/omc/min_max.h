/**
* @file    min_max.h
* Implementation of min, max, clamp templates
*/
// Copyright (c) 2013-2016 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#pragma once

// Undefine min and max macros if defined.
// Note: MFC defines min/max macros by default, so are defined in Modgen models.
#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif

#include <type_traits>
#include <algorithm> // for std::min and std::max

// Replacement versions in global namespace of std:min, std::max, and std::clamp
// which enable type conversion of arguments to a common shared type.
// Specializations of common_type<> for several ompp types enables 
// their use in min, max, clamp. They are declared in their respective header files.

template <typename T, typename U, typename CT = typename std::common_type<T, U>::type>
inline CT min(T a, U b)
{
    // explicit casts are used below for user-defined types
    return (CT)a < (CT)b ? (CT)a : (CT)b;
}

template <typename T, typename U, typename CT = typename std::common_type<T, U>::type>
inline CT max(T a, U b)
{
    // explicit casts are used below for user-defined types
    return (CT)a < (CT)b ? (CT)b : (CT)a;
}

template <typename T, typename U, typename V, typename CT = typename std::common_type<T, U, V>::type>
inline CT clamp(T v, U lo, V hi)
{
    // explicit casts are used below for user-defined types
    if ((CT)lo > (CT)v) return (CT)lo;
    else if ((CT)hi < (CT)v) return (CT)hi;
    else return (CT)v;
}
