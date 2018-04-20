/**
* @file    min_max.h
* Implementation of min, max, clamp templates
*/
// Copyright (c) 2013-2016 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#pragma once

#include <type_traits>
#include <algorithm> // for std::min and std::max

// enable std templates (arguments have identical types)
using std::min;
using std::max;
//using std::clamp;

#define OVERLOAD_MIN_MAX 1
#if OVERLOAD_MIN_MAX // being tested
//
// min/max/clamp overload functions with explicit argument types
//

// minimum of two doubles
inline const double min(const double& a, const double& b)
{
    return (b < a ? b : a);
}

// maximum of two doubles
inline double max(const double& a, const double& b)
{
    return (a < b ? b : a);
}

// clamped value of a double
inline const double clamp(const double &v, const double &lo, const double &hi)
{
    return (v < lo ? lo : v > hi ? hi : v);
}

#endif // OVERLOAD_MIN_MAX // being tested



//
// min/max/clamp functions with explicit argument types passed value
//

// minimum of two doubles
inline const double mind(const double& a, const double& b)
{
    return (b < a ? b : a);
}

// maximum of two doubles
inline double maxd(const double& a, const double& b)
{
    return (a < b ? b : a);
}

// clamped value of a double
inline const double clampd(const double &v, const double &lo, const double &hi)
{
    return (v < lo ? lo : v > hi ? hi : v);
}

// minimum of two ints
inline const int mini(const int& a, const int& b)
{
    return (b < a ? b : a);
}

// maximum of two ints
inline const int maxi(const int& a, const int& b)
{
    return (a < b ? b : a);
}

// clamped value of an int
inline const int clampi(const int& v, const int& lo, const int& hi)
{
    return (v < lo ? lo : v > hi ? hi : v);
}


#define MIXED_MODE_MIN_MAX 0
#if MIXED_MODE_MIN_MAX // being tested

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