/**
* @file    GroupSymbol.h
* Declarations for the GroupSymbol class.
*/
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

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
        , is_hidden(false)
        , pp_group_id(-1)
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
        , is_hidden(false)
        , pp_group_id(-1)
    {
    }

    void post_parse(int pass);

    /**
     * Return true if group contains any circular references.
     */
    bool is_circular() const;

    /**
     * Recursive helper function for is_circular.
     */
    bool is_circular_helper(const string & name) const;

    /**
     * Return list with all groups expanded.
     */
    list<Symbol *> expanded_list() const;

    /**
     * Return a group, expanded.
     */
    list<Symbol *> expand_group() const;

    /**
     * List of symbols.
     */
    list<Symbol **> symbol_list;

    /**
     * List of symbols (post-parse)
     */
    list<Symbol *> pp_symbol_list;

    /**
     * True if the group is hidden
     */
    bool is_hidden;

    /**
     * Numeric identifier. Used for communicating with metadata API.
     */
    int pp_group_id;
};
