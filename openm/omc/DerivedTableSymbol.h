/**
* @file    DerivedTableSymbol.h
* Declarations for the DerivedTableSymbol class.
*/
// Copyright (c) 2013-2014 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once
#include "Symbol.h"
#include "DerivedTablePlaceholderSymbol.h"

/**
* DerivedTableSymbol.
*
*/
class DerivedTableSymbol : public Symbol
{
private:
    typedef Symbol super;

public:
    bool is_base_symbol() const { return false; }

    /**
    * Morphing constructor.
    *
    * @param [in,out]  sym The symbol to be morphed.
    */
    DerivedTableSymbol(Symbol *sym, yy::location decl_loc = yy::location())
        : Symbol(sym, decl_loc)
    {
    }

    void post_parse(int pass);

    /**
     * The placeholders in the derived table.
     */
    list<DerivedTablePlaceholderSymbol *> pp_placeholders;

};
