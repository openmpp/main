/**
* @file    GroupSymbol.h
* Declarations for the GroupSymbol class.
*/
// Copyright (c) 2013-2014 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once
#include <list>
#include "Symbol.h"

/**
* GroupSymbol.
*
*/
class GroupSymbol : public Symbol
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
    GroupSymbol(Symbol *sym, yy::location decl_loc = yy::location())
        : Symbol(sym, decl_loc)
    {
    }

    /**
     * Constructor by name.
     *
     * @param unique_name The symbol name
     * @param decl_loc    (Optional) the declaration location.
     */
    GroupSymbol(const string unique_name, yy::location decl_loc = yy::location())
        : Symbol(unique_name, decl_loc)
    {
    }

    void post_parse(int pass);

    /**
     * List of symbols.
     */
    list<Symbol **> symbol_list;

    /**
     * List of symbols (post-parse)
     */
    list<Symbol *> pp_symbol_list;
};
