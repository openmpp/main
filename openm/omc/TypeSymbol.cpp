/**
* @file    TypeSymbol.cpp
* Definitions for the TypeSymbol class.
*/
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#include <cassert>
#include "TypeSymbol.h"
#include "NumericSymbol.h"
#include "BoolSymbol.h"
#include "StringTypeSymbol.h"
#include "ClassificationSymbol.h"
#include "RangeSymbol.h"
#include "PartitionSymbol.h"
#include "TimeSymbol.h"
#include "RealSymbol.h"

using namespace std;

bool TypeSymbol::is_bool()
{
    if (dynamic_cast<BoolSymbol *>(this)) {
        return true;
    }
    else {
        return false;
    }
}

bool TypeSymbol::is_string()
{
    if (dynamic_cast<StringTypeSymbol *>(this)) {
        return true;
    }
    else {
        return false;
    }
}

bool TypeSymbol::is_classification()
{
    if (dynamic_cast<ClassificationSymbol *>(this)) {
        return true;
    }
    else {
        return false;
    }
}

bool TypeSymbol::is_range()
{
    if (dynamic_cast<RangeSymbol *>(this)) {
        return true;
    }
    else {
        return false;
    }
}

bool TypeSymbol::is_partition()
{
    if (dynamic_cast<PartitionSymbol *>(this)) {
        return true;
    }
    else {
        return false;
    }
}

bool TypeSymbol::is_time()
{
    if (dynamic_cast<TimeSymbol *>(this)) {
        return true;
    }
    else {
        return false;
    }
}

bool TypeSymbol::is_numeric_or_bool()
{
    if (dynamic_cast<NumericSymbol *>(this) || dynamic_cast<BoolSymbol *>(this)) {
        return true;
    }
    else {
        assert(dynamic_cast<ClassificationSymbol *>(this)
            || dynamic_cast<RangeSymbol *>(this)
            || dynamic_cast<PartitionSymbol *>(this)
            || dynamic_cast<StringTypeSymbol *>(this)); // grammar guarantee
        return false;
    }
}

bool TypeSymbol::is_enumeration()
{
    if (dynamic_cast<ClassificationSymbol *>(this)
     || dynamic_cast<BoolSymbol *>(this)
     || dynamic_cast<RangeSymbol *>(this)
     || dynamic_cast<PartitionSymbol *>(this)) {
        return true;
    }
    else {
        return false;
    }
}

bool TypeSymbol::is_floating() const
{
    if (auto ns = dynamic_cast<const NumericSymbol *>(this)) {
        if (ns->name == "real" || ns->name == "float" || ns->name == "double" || ns->name == "ldouble") {
            return true;
        }
        if (auto ts = dynamic_cast<const TimeSymbol *>(this)) {
            auto tt = ts->time_type;
            if (tt == token::TK_float || tt == token::TK_double || tt == token::TK_ldouble ) {
                return true;
            }
        }
    }
    return false;
}

TypeSymbol * TypeSymbol::summing_type()
{

    if (is_floating()) {
        // summing type is real
        return RealSymbol::find();
    }
    else {
        // summing type is integer
        auto *sym = NumericSymbol::find(token::TK_integer);
        assert(sym);  // Initialization guarantee
        return sym;
    }
}


string TypeSymbol::exposed_type()
{
    if (auto cs = dynamic_cast<ClassificationSymbol *>(this)) {
        // a classification exposes an enum
        string typ = cs->enum_name();
        return true ? "int" : typ; // for testing - expose int or expose enum?
    }
    else if (dynamic_cast<RangeSymbol *>(this)) {
        // a range exposes an int
        return "int";
    }
    else if (dynamic_cast<PartitionSymbol *>(this)) {
        // a partition exposes an int
        return "int";
    }
    else if (auto ts = dynamic_cast<TimeSymbol *>(this)) {
        if (ts->is_wrapped()) {
            // fixed_precision_float<...>
            // a Time type of float, double, etc. exposes the floating point type
            return (Symbol::token_to_string(ts->time_type));
        }
        else {
            // not wrapped - no additional exposed type
            return "void";
        }
    }
    else {
        // not wrapped - no additional exposed type
        return "void";
    }
}


int TypeSymbol::next_type_id = 0;
