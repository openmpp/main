/**
* @file    ParameterSymbol.h
* Declarations for the ParameterSymbol class.
*/
// Copyright (c) 2013-2014 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once
#include "Symbol.h"
#include "TypedefTypeSymbol.h"

/**
* ParameterSymbol.
*/
class ParameterSymbol : public Symbol
{
private:
    typedef Symbol super;

public:
    ParameterSymbol(Symbol *sym, Symbol *type_symbol, yy::location decl_loc = yy::location())
        : Symbol(sym, decl_loc)
        , type_symbol(type_symbol->get_rpSymbol())
    {
    }

    void post_parse(int pass);

    CodeBlock cxx_declaration_global();

    CodeBlock cxx_definition_global();

    void populate_metadata(openm::MetaModelHolder & metaRows);

    // members
    
    /**
    * Reference to pointer to type symbol
    *
    * Stable to symbol morphing during parse phase.
    * Indirection is required because the type symbol might be a classification
    * or range which has not yet been declared.
    */

    Symbol*& type_symbol;


    /**
     * Direct pointer to type symbol for post-parse convenience.
     */

    TypeSymbol *pp_type_symbol;


    /**
     * true if the data is enum., i.e. a classification or a range.
     */

    bool pp_is_enum;


    /**
     * Numeric identifier. Used for communicating with metadata API.
     */

    int pp_parameter_id;
};

