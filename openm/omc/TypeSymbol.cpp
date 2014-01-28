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

using namespace std;

bool TypeSymbol::numeric_or_bool()
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

int TypeSymbol::next_type_id = 0;
