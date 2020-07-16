/**
* @file    Literal.cpp
* Definitions for the Literal classes.
*/
// Copyright (c) 2013-2020 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#include <regex>
#include "libopenm/common/omHelper.h"
#include "Literal.h"

using namespace std;

// regex to validate integer number
static const regex intRx("(^[+-]?)([0-9]+$)");

// regex to validate float number: ^[-+]?(([0-9]+(\.?)([0-9]+)?)|(\.[0-9]+))([eE][-+]?[0-9]+)?$
// including INF or NAN constants: ^[-+]?((([0-9]+(\.?)([0-9]+)?)|(\.[0-9]+))([E][-+]?[0-9]+)?$)|(INFINITY|INF|NAN)
// hexadecimal notation and F or L suffixes not supported
static const regex floatRx("^[-+]?((([0-9]+(\\.?)([0-9]+)?)|(\\.[0-9]+))([E][-+]?[0-9]+)?$)|(INFINITY|INF|NAN)", regex_constants::icase);

// static
bool IntegerLiteral::is_valid_literal(const char * i_value) 
{
    return i_value != nullptr && regex_match(i_value, intRx);
}

// static
bool FloatingPointLiteral::is_valid_literal(const char * i_value)
{
    return i_value != nullptr && regex_match(i_value, floatRx);
}

// static
bool BooleanLiteral::is_valid_literal(const char * i_value)
{
    return
        i_value != nullptr &&
        (!strcmp(i_value, "false") || !strcmp(i_value, "FALSE") ||
            !strcmp(i_value, "true") || !strcmp(i_value, "TRUE"));
}

// static
bool TimeLiteral::is_valid_literal(const char * i_value)
{
    return
        i_value != nullptr &&
        (!strcmp(i_value, "time_undef") || !strcmp(i_value, "TIME_UNDEF") || 
            !strcmp(i_value, "time_infinite") || !strcmp(i_value, "TIME_INFINITE") ||
            FloatingPointLiteral::is_valid_literal(i_value));
}
