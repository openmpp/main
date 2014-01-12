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
    ParameterSymbol(Symbol *sym, token_type type, yy::location decl_loc = yy::location())
        : Symbol(sym, decl_loc)
        , type(type)
    {
    }

    void post_parse(int pass);

    CodeBlock cxx_declaration_global();

    CodeBlock cxx_definition_global();

    void populate_metadata(openm::MetaModelHolder & metaRows);

    // members
    token_type type;

    /**
    * Numeric identifier.
    * Used for communicating with metadata API.
    */

    int pp_parameter_id;
};

