/**
* @file    BoolSymbol.h
* Declarations for the BoolSymbol class.
*/
// Copyright (c) 2013-2014 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once
#include "EnumerationSymbol.h"
#include "TypeSymbol.h"

using namespace std;
using namespace openm;

/**
* BoolSymbol.
*/
class BoolSymbol : public EnumerationSymbol
{
private:
    typedef EnumerationSymbol super;

public:
    bool is_base_symbol() const { return false; }

    /**
    * Constructor for the bool enumeration symbol
    */

    BoolSymbol()
        : EnumerationSymbol(token_to_string(token::TK_bool), token::TK_bool, kind_of_type::logical_type)
    {
    }

    void post_parse(int pass);
};

