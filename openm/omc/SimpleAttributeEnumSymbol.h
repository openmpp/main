/**
* @file    SimpleAttributeEnumSymbol.h
* Declarations for the SimpleAttributeEnumSymbol class.
*/
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#pragma once
#include "SimpleAttributeSymbol.h"
#include "EnumeratorSymbol.h"

using namespace std;


/**
 * SimpleAttributeEnumSymbol.
 * 
 * A specialization of SimpleAttributeSymbol with an enumerator initializer.
 */

class SimpleAttributeEnumSymbol : public SimpleAttributeSymbol
{
private:
    typedef SimpleAttributeSymbol super;

public:
    bool is_base_symbol() const { return false; }

    SimpleAttributeEnumSymbol(Symbol* sym, const Symbol* agent, const Symbol* type, Symbol* enumerator, omc::location decl_loc = omc::location())
        : SimpleAttributeSymbol(sym, agent, type, nullptr, decl_loc)
        , enumerator(enumerator->stable_rp())
        , pp_enumerator(0)
    {
    }

    ~SimpleAttributeEnumSymbol()
    {
    }

    void post_parse(int pass);

    /**
     * Gets the initial value for the attribute.
     *
     * @param type_default true to return the default value for the type.
     *
     * @return The initial value as a string.
     */
    string initialization_value(bool type_default) const;

    // members

    /**
     * Reference to pointer to enumerator
     * 
     * Stable to symbol morphing during parse phase.
     */
    Symbol*& enumerator;

    /**
     * Direct pointer to enumerator.
     * 
     * Valid post-parse.
     */
    EnumeratorSymbol *pp_enumerator;
};

