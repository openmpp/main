/**
* @file    ParameterSymbol.h
* Declarations for the ParameterSymbol class.
*/
// Copyright (c) 2013 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once
#include "Symbol.h"

/**
* ParameterSymbol.
*/
class ParameterSymbol : public Symbol
{
private:
    typedef Symbol super;

public:
    ParameterSymbol(Symbol *sym, token_type type)
        : Symbol(sym)
        , type(type)
    {
    }

    void post_parse(int pass);

    CodeBlock cxx_declaration();

    CodeBlock cxx_definition();

    // members
    token_type type;

    /**
    * Numeric identifier.
    * Used for communicating with metadata API.
    */

    int pp_numeric_id;
};

