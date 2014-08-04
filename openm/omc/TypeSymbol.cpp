/**
* @file    TypeSymbol.cpp
* Definitions for the TypeSymbol class.
*/
// Copyright (c) 2013-2014 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#include <cassert>
#include "TypeSymbol.h"
#include "NumericSymbol.h"
#include "BoolSymbol.h"
#include "ClassificationSymbol.h"
#include "RangeSymbol.h"
#include "PartitionSymbol.h"
#include "TimeSymbol.h"

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
            || dynamic_cast<PartitionSymbol *>(this)); // grammar guarantee
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

string TypeSymbol::wrapped_type()
{
    if (dynamic_cast<ClassificationSymbol *>(this)
     || dynamic_cast<RangeSymbol *>(this)
     || dynamic_cast<PartitionSymbol *>(this)) {
        auto es = dynamic_cast<EnumerationSymbol *>(this);
        assert(es);
        return (Symbol::token_to_string(es->storage_type));
    }
    else if (dynamic_cast<TimeSymbol *>(this)) {
        auto ts = dynamic_cast<TimeSymbol *>(this);
        assert(ts);
        if (ts->is_wrapped()) {
            return (Symbol::token_to_string(ts->time_type));
        }
        else {
            return "void";
        }
    }
    else {
        return "void";
    }
}


int TypeSymbol::next_type_id = 0;
