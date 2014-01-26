/**
* @file    BoolEnumTypeSymbol.h
* Declarations for the BoolEnumTypeSymbol class.
*/
// Copyright (c) 2013-2014 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once
#include "EnumTypeSymbol.h"
#include "TypeSymbol.h"

using namespace std;
using namespace openm;

/**
* BoolEnumTypeSymbol.
*/
class BoolEnumTypeSymbol : public EnumTypeSymbol
{
private:
    typedef EnumTypeSymbol super;

public:

    /**
     * Constructor for the bool enumeration symbol
     */

    BoolEnumTypeSymbol()
        : EnumTypeSymbol(token_to_string(token::TK_bool), token::TK_bool, kind_of_type::logical_type)
    {
    }

    void post_parse(int pass);
};

