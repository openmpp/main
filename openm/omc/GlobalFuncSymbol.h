/**
* @file    GlobalFuncSymbol.h
* Declarations for the GlobalFuncSymbol class.
*/
// Copyright (c) 2013-2014 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once
#include "Symbol.h"

/**
* GlobalFuncSymbol.
*
*/
class GlobalFuncSymbol : public Symbol
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
    GlobalFuncSymbol(Symbol *sym, yy::location decl_loc = yy::location())
        : Symbol(sym, decl_loc)
    {
    }

    /**
    * Constructor by name
    *
    * @param [in,out]  sym The symbol to be morphed.
    */
    GlobalFuncSymbol(const string name, yy::location decl_loc = yy::location())
        : Symbol(name, decl_loc)
    {
    }

    void post_parse(int pass);
};
