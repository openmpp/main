/**
* @file    ClassificationEnumeratorSymbol.h
* Declarations for the ClassificationEnumeratorSymbol class.
*/
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#pragma once
#include <cassert>
#include "EnumeratorSymbol.h"

class EnumerationSymbol;

using namespace std;

class ClassificationEnumeratorSymbol : public EnumeratorSymbol
{
private:
    typedef EnumeratorSymbol super;

public:
    bool is_base_symbol() const override { return false; }

    explicit ClassificationEnumeratorSymbol(Symbol *sym, const Symbol *enumeration, int ordinal, yy::location decl_loc = yy::location())
        : EnumeratorSymbol(sym, enumeration, ordinal, decl_loc)
    {
    }

	string db_name() const
	{
		// The 'name' in the data store is the same as the classification level in model code
		return name;
	};

};


