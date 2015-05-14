/**
* @file    DerivedTableSymbol.h
* Declarations for the DerivedTableSymbol class.
*/
// Copyright (c) 2013-2014 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once
#include <list>
#include "Symbol.h"
#include "TableSymbol.h"

class CodeBlock;

/**
* DerivedTableSymbol.
*
*/
class DerivedTableSymbol : public TableSymbol
{
private:
    typedef TableSymbol super;

public:
    bool is_base_symbol() const { return false; }

    /**
    * Morphing constructor.
    *
    * @param [in,out]  sym The symbol to be morphed.
    */
    DerivedTableSymbol(Symbol *sym, yy::location decl_loc = yy::location())
        : TableSymbol(sym, decl_loc)
    {
    }

    void post_parse(int pass);

    CodeBlock cxx_declaration_global();

    CodeBlock cxx_definition_global();

    /**
     * The C++ initializer for the table
     * 
     * This is an expression list used to initialize the table at construction.
     * The expression list does not include the enclosing parentheses used for construction.
     * 
     * Example: {2, 5} for a 2x5 table
     * 
     * @return A string.
     */
    string cxx_initializer() const;
};
