/**
* @file    EnumeratorSymbol.h
* Declarations for the EnumeratorSymbol class.
*/
// Copyright (c) 2013-2014 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once
#include "Symbol.h"

class EnumerationSymbol;

using namespace std;

class EnumeratorSymbol : public Symbol
{
private:
    typedef Symbol super;

public:
    bool is_base_symbol() const { return false; }

    EnumeratorSymbol(Symbol *sym, const Symbol *enumeration, int ordinal, yy::location decl_loc = yy::location())
        : Symbol(sym, decl_loc)
        , enumeration(enumeration->get_rpSymbol())
        , ordinal(ordinal)
        , pp_enumeration(nullptr)
    {
    }

    EnumeratorSymbol(const string unm, const Symbol *enumeration, int ordinal, yy::location decl_loc = yy::location())
        : Symbol(unm, decl_loc)
        , enumeration(enumeration->get_rpSymbol())
        , ordinal(ordinal)
        , pp_enumeration(nullptr)
    {
    }

    void post_parse(int pass);


    /**
    * Reference to pointer to enumeration.
    *
    * Stable to symbol morhing during parse phase.
    */

    Symbol*& enumeration;


    /**
    * ordinal giving order of level within enumeration
    */

    int ordinal;


    /**
     * Direct pointer to enumeration.
     * 
     * Set during post-parse for convenience.
     */

    EnumerationSymbol *pp_enumeration;
};


