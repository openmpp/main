/**
* @file    TypeSymbol.cpp
* Definitions for the TypeSymbol class.
*/
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#include <cassert>
#include "TypeSymbol.h"
#include "UnknownTypeSymbol.h"
#include "NumericSymbol.h"
#include "BoolSymbol.h"
#include "StringTypeSymbol.h"
#include "ClassificationSymbol.h"
#include "RangeSymbol.h"
#include "PartitionSymbol.h"
#include "TimeSymbol.h"
#include "RealSymbol.h"

using namespace std;

bool TypeSymbol::is_unknown() const
{
    if (dynamic_cast<const UnknownTypeSymbol *>(this)) {
        return true;
    }
    else {
        return false;
    }
}

bool TypeSymbol::is_bool() const
{
    if (dynamic_cast<const BoolSymbol *>(this)) {
        return true;
    }
    else {
        return false;
    }
}

bool TypeSymbol::is_string() const
{
    if (dynamic_cast<const StringTypeSymbol *>(this)) {
        return true;
    }
    else {
        return false;
    }
}

bool TypeSymbol::is_classification() const
{
    if (dynamic_cast<const ClassificationSymbol *>(this)) {
        return true;
    }
    else {
        return false;
    }
}

bool TypeSymbol::is_range() const
{
    if (dynamic_cast<const RangeSymbol *>(this)) {
        return true;
    }
    else {
        return false;
    }
}

bool TypeSymbol::is_partition() const
{
    if (dynamic_cast<const PartitionSymbol *>(this)) {
        return true;
    }
    else {
        return false;
    }
}

bool TypeSymbol::is_time() const
{
    if (dynamic_cast<const TimeSymbol *>(this)) {
        return true;
    }
    else {
        return false;
    }
}

bool TypeSymbol::is_numeric_or_bool() const
{
    if (dynamic_cast<const NumericSymbol *>(this) || dynamic_cast<const BoolSymbol *>(this)) {
        return true;
    }
    else {
        assert(dynamic_cast<const ClassificationSymbol *>(this)
            || dynamic_cast<const RangeSymbol *>(this)
            || dynamic_cast<const PartitionSymbol *>(this)
            || dynamic_cast<const StringTypeSymbol *>(this)); // grammar guarantee
        return false;
    }
}

bool TypeSymbol::is_enumeration() const
{
    if (dynamic_cast<const ClassificationSymbol *>(this)
     || dynamic_cast<const BoolSymbol *>(this)
     || dynamic_cast<const RangeSymbol *>(this)
     || dynamic_cast<const PartitionSymbol *>(this)) {
        return true;
    }
    else {
        return false;
    }
}

bool TypeSymbol::is_floating() const
{
    if (is_numeric_floating()) {
        return true;
    }
    if (auto ts = dynamic_cast<const TimeSymbol *>(this)) {
        auto tt = ts->time_type;
        if (tt == token::TK_float || tt == token::TK_double || tt == token::TK_ldouble ) {
            return true;
        }
    }
    return false;
}

bool TypeSymbol::is_numeric_floating() const
{
    if (auto ns = dynamic_cast<const NumericSymbol *>(this)) {
        if (ns->name == "real" || ns->name == "float" || ns->name == "double" || ns->name == "ldouble") {
            return true;
        }
    }
    return false;
}

bool TypeSymbol::is_numeric_integer() const
{
    if (auto ns = dynamic_cast<const NumericSymbol *>(this)) {
        return !is_numeric_floating() && !is_time();
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
            // fixed_precision<...>
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
