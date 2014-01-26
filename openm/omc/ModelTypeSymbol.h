/**
* @file    ModelTypeSymbol.h
* Declarations for the ModelTypeSymbol class.
*/
// Copyright (c) 2013-2014 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once
#include <cassert>
#include "Symbol.h"

/**
* Symbol for the model type information
*
* A single instance of ModelTypeSymbol exists in the symbol table, with name 'model_type'.
* A default ModelTypeSymbol with value 'case_based' is created at initialization.
* This default ModelTypeSymbol will be morphed if a 'model_type' statement
* is encountered in the model source code.
*/
class ModelTypeSymbol : public Symbol
{
private:
    typedef Symbol super;

public:
    bool is_base_symbol() const { return false; }

    /**
     * Constructor.
     *
     * @param value The token for the model type, e.g. token::KW_case_based.
     */

    ModelTypeSymbol(token_type value, bool just_in_time = false, yy::location decl_loc = yy::location())
        : Symbol(token_to_string(token::TK_model_type), decl_loc)
        , value(value)
        , just_in_time(just_in_time)
    {
        // compiler guarantee
        assert(value == token::TK_case_based || value == token::TK_time_based);
    }

    CodeBlock cxx_definition_global();

    void populate_metadata(openm::MetaModelHolder & metaRows);


    /**
     * The model type
     *
     *  This can be either TK_case_based or TK_time_based.
     */

    token_type value;


    /**
     * Value of just-in-time option
     * 
     * True if active, false if inactive
     */

    bool just_in_time;
};

