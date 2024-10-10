/**
* @file    EnumeratorSymbol.h
* Declarations for the EnumeratorSymbol class.
*/
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#pragma once
#include "Symbol.h"
#include "EnumerationWithEnumeratorsSymbol.h"

using namespace std;

class EnumeratorSymbol : public Symbol
{
private:
    typedef Symbol super;

public:
    bool is_base_symbol() const override { return false; }

    EnumeratorSymbol(Symbol *sym, const Symbol *enumeration, int ordinal, omc::location decl_loc = omc::location())
        : Symbol(sym, decl_loc)
        , enumeration(enumeration->stable_rp())
        , pp_enumeration(nullptr)
        , ordinal(ordinal)
    {
    }

    EnumeratorSymbol(const string unm, const Symbol *enumeration, int ordinal, omc::location decl_loc = omc::location())
        : Symbol(unm, decl_loc)
        , enumeration(enumeration->stable_rp())
        , pp_enumeration(nullptr)
        , ordinal(ordinal)
    {
    }

    void post_parse(int pass) override;

	/**
	* Gets the short 'name' associated with the enumerator in the data store.
	*
	* @return A string.
	*/
	virtual string db_name() const = 0;

    bool is_published() const override
    {
        // enumerator is published if parent enumeration is published
        return pp_enumeration->is_published();
    }

    /**
    * Reference to pointer to enumeration.
    *
    * Stable to symbol morhing during parse phase.
    */

    Symbol*& enumeration;


    /**
     * Direct pointer to enumeration.
     * 
     * Set during post-parse for convenience.
     */

    EnumerationWithEnumeratorsSymbol *pp_enumeration;


    /**
    * ordinal giving order of level within enumeration
    */

    int ordinal;
};


