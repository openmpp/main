/**
* @file    ModelTypeSymbol.h
* Declarations for the ModelTypeSymbol class.
*/
// Copyright (c) 2013 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once
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

    /**
     * Constructor.
     *
     * @param value The token for the model type, e.g. token::KW_case_based.
     */

    ModelTypeSymbol(token_type value)
        : Symbol(token_to_string(token::TK_model_type))
        , value(value)
    {
    }

    CodeBlock cxx_definition_global();

    void populate_metadata(openm::MetaModelHolder & metaRows);

    /** The model type */
    token_type value;
};

