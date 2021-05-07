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

// Obscurely, Modgen inserts a trailing -1 argument in marked up code,
// which explains why it is also inserted here by the cover macros.
// The following macros use compiler specific behaviour,
// but in any case only apply to Modgen compilation under MSVC.
// The ## operator removes the preceding , if no arguments are supplied, but is non-standard.

#define FixedGetTableValue(table_measure, ...) \
    UNDEF_VALUE_to_NaN(GetTableValue(table_measure, ## __VA_ARGS__, -1))

#define FixedSetTableValue(table_measure, value, ...) \
    SetTableValue(table_measure, NaN_to_UNDEF_VALUE(value), ## __VA_ARGS__, -1)

// Extension to Modgen derived table API
inline bool TablePresent(const char* table_name)
{
    return true;
}

#else // defined(OPENM)

// The use of ## to suppress the preceding comma is non-standard.
// It should be reworked to use the standards compliant __VA_OPT__ mechanism in C++20
// (although MSVC turns off the standards compliant preprocessor by default 2021-05-06).
// OTOH, this whole derived table mechanism in Modgen should be replaced.
#define FixedGetTableValue(table_measure, ...) \
    GetTableValue(table_measure, ##__VA_ARGS__)
#define FixedSetTableValue(table_measure, value, ...) \
    SetTableValue(table_measure, value, ##__VA_ARGS__)

/** Is table present in the model (not suppressed) */
inline bool TablePresent(const char* table_name)
{
    return 0 != om_table_names.count(table_name);
}

#endif

