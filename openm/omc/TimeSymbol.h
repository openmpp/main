/**
* @file    TimeSymbol.h
* Declarations for the TimeSymbol class.
*/
// Copyright (c) 2013-2014 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once
#include "NumericSymbol.h"

class TimeSymbol : public NumericSymbol
{
private:
    typedef NumericSymbol super;

public:
    bool is_base_symbol() const { return false; }

    /**
    * Constructor.
    *
    * @param kw1  Keyword #1 for the associated typedef, e.g. token::KW_double.
    */

    TimeSymbol(token_type kw1)
        : NumericSymbol(token::TK_Time, kw1, "0")
    {
    }

    CodeBlock cxx_declaration_global();

    CodeBlock cxx_definition_global();

};
