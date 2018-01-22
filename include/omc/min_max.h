/**
* @file    min_max.h
* Implementation of min, max, clamp templates
*/
// Copyright (c) 2013-2016 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#pragma once

#include <type_traits>
#include <algorithm> // for std::min and std::max

//
// min/max/clamp functions with explicit argument types passed value
//

// minimum of two doubles
inline double mind(double a, double b)
{
    return (b < a ? b : a);
}

// maximum of two doubles
inline double maxd(double a, double b)
{
    return (a < b ? b : a);
}

// clamped value of a double
inline double clampd(double v, double lo, double hi)
{
    return (v < lo ? lo : v > hi ? hi : v);
}

// minimum of two ints
inline int mini(int a, int b)
{
    return (b < a ? b : a);
}

// maximum of two ints
inline int maxi(int a, int b)
{
    return (a < b ? b : a);
}

// clamped value of an int
inline int clampi(int v, int lo, int hi)
{
    return (v < lo ? lo : v > hi ? hi : v);
}



#if true // being tested

//
// min/max/clamp templates for mixed-mode argument types
//

// Replacement versions in global namespace of std:min, std::max, and std::clamp
// which enable type conversion of arguments to a common shared type.

// Specializations of common_type<> for several ompp types enables 
// their use in min, max, clamp. They are declared in their respective header files.

template <typename T, typename U, typename CT = typename std::common_type<T, U>::type>
inline CT min(const T& a, const U& b)
{
    // explicit casts are used below to invoke user-defined conversion for user-defined types
    // return by value
    return (CT)a < (CT)b ? (CT)a : (CT)b;
}

template <typename T, typename U, typename CT = typename std::common_type<T, U>::type>
inline CT max(const T& a, const U& b)
{
    // explicit casts are used below to invoke user-defined conversion for user-defined types
    // return by value
    return (CT)a < (CT)b ? (CT)b : (CT)a;
}

template <typename T, typename U, typename V, typename CT = typename std::common_type<T, U, V>::type>
inline CT clamp(const T& v, const U& lo, const V& hi)
{
    // explicit casts are used below to invoke user-defined conversion for user-defined types
    // return by value
    return ((CT)v < (CT)lo ? (CT)lo : (CT)v > (CT)hi ? (CT)hi : (CT)v);
}
#endif