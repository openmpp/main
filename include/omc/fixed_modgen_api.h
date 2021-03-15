/**
* @file    fixed_modgen_api.h
* Implementation of FixedGetTableValue and FixedSetTableValue
*/
// Copyright (c) 2021 OpenM++ Contributors
// This code is licensed under the MIT license (see LICENSE.txt for details)

#pragma once

#include <limits>
#include <cmath>

#if defined(MODGEN)

// Helper function to convert Modgen UNDEF_VALUE to quiet NaN
inline double UNDEF_VALUE_to_NaN(double x)
{
    return (UNDEF_VALUE == x) ? std::numeric_limits<double>::quiet_NaN() : x;
}

// Helper function to convert quiet NaN to Modgen UNDEF_VALUE
inline double NaN_to_UNDEF_VALUE(double x)
{
    return (std::isnan(x)) ? UNDEF_VALUE : x;
}

#define FixedGetTableValue(table_cell, ...) \
    UNDEF_VALUE_to_NaN(GetTableValue(table_cell, ##__VA_ARGS__))
#define FixedSetTableValue(table_cell, value, ...) \
    SetTableValue(table_cell, NaN_to_UNDEF_VALUE(value), ##__VA_ARGS__)

#else // defined(OPENM)

#define FixedGetTableValue(table_cell, ...) \
    GetTableValue(table_cell, ##__VA_ARGS__)
#define FixedSetTableValue(table_cell, value, ...) \
    SetTableValue(table_cell, value, ##__VA_ARGS__)

#endif

