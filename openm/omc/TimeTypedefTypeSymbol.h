/**
* @file    TimeTypedefTypeSymbol.h
* Declarations for the TimeTypedefTypeSymbol class.
*/
// Copyright (c) 2013-2014 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once
#include "TypedefTypeSymbol.h"

class TimeTypedefTypeSymbol : public TypedefTypeSymbol
{
private:
    typedef TypedefTypeSymbol super;

public:

    /**
    * Constructor.
    *
    * @param kw1  Keyword #1 for the associated typedef, e.g. token::KW_double.
    */

    TimeTypedefTypeSymbol(token_type kw1)
        : TypedefTypeSymbol(token::TK_Time, kw1, "0")
    {
    }

    CodeBlock cxx_declaration_global();

    CodeBlock cxx_definition_global();

};
