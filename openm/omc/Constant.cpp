/**
* @file    Constant.cpp
* Definitions for the Constant class.
*/
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#include <cassert>
#include "Constant.h"
#include "EnumeratorSymbol.h"
#include "TypeSymbol.h"
#include "Literal.h"

using namespace std;

const string Constant::value() const
{
    if (is_literal) {
        return literal->value();
    }
    else {
        return (*enumerator)->name;
    }
}

const string Constant::value_as_name() const
{
    if (is_literal) {
        string work = literal->value();
        for (auto &ch : work) {
            if (ch == '.') ch = 'o';
            if (ch == '-') ch = 'm';
        }
        return work;
    }
    else {
        return (*enumerator)->name;
    }
}

bool Constant::is_valid_constant(const TypeSymbol &type) const
{
    return type.is_valid_constant(*this);
}

string Constant::format_for_storage(const TypeSymbol &type) const
{
    return type.format_for_storage(*this);
}
