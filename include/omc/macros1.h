/**
* @file    macros1.h
* Macros and templates available to all models (late).
*/
// Copyright (c) 2013-2016 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#include <type_traits>
#include "libopenm/omLog.h"

using namespace std;

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

/** LT localisation function: return is temporary const char* and must be copied to avoid memory violation crash. */
#define LT(sourceMessage) ((theLog->getMessage(sourceMessage)).c_str())
