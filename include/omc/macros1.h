/**
* @file    macros1.h
* Macros and templates available to all models (late).
*/
// Copyright (c) 2013-2016 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#include <type_traits>

// Replacement versions of std:min and std::max which enable type conversion of arguments
// to a common shared type.


template <typename T, typename U>
inline typename std::common_type<T, U>::type min(T a, U b)
{
	return a < b ? a : b;
}

template <typename T, typename U>
inline typename std::common_type<T, U>::type max(T a, U b)
{
	return a < b ? b : a;
}

//TODO Temporary cover for LT localisation function
#define LT(x) x
