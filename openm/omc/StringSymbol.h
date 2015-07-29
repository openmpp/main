/**
* @file    StringSymbol.h
* Declarations for the StringSymbol class.
*/
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#pragma once
#include "Symbol.h"

/**
* StringSymbol.
*
*/
class StringSymbol : public Symbol
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
    StringSymbol(Symbol *sym, yy::location decl_loc = yy::location())
        : Symbol(sym, decl_loc)
    {
    }

    void post_parse(int pass);
};
