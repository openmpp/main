/**
* @file    EnumeratorSymbol.h
* Declarations for the EnumeratorSymbol class.
*/
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#pragma once
#include "Symbol.h"

class EnumerationWithEnumeratorsSymbol;

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


    ///**
    //* Heuristically-generated short name for the enumerator
    //*/
    //string heuristic_short_name(void) const;

    ///**
    //* Short name of the enumerator
    //*/
    //string short_name;

    ///**
    //* Short name of the enumerator (generated default name)
    //*/
    //string short_name_default;

    ///**
    //* Short name of the enumerator (provided in model source)
    //*/
    //string short_name_explicit;

	/**
	* Gets the short 'name' associated with the enumerator in the data store.
	*
	* @return A string.
	*/
	virtual string db_name() const = 0;


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


