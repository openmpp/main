/**
* @file    SimpleAttributeSymbol.h
* Declarations for the SimpleAttributeSymbol class.
*/
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#pragma once
#include "AttributeSymbol.h"

using namespace std;


/**
 * SimpleAttributeSymbol.
 * 
 * A symbol of this class represents an attribute whose value can be modified by C++ code in an
 * event implement function.
 */

class SimpleAttributeSymbol : public AttributeSymbol
{
private:
    typedef AttributeSymbol super;

public:
    bool is_base_symbol() const { return false; }

    SimpleAttributeSymbol(Symbol *sym, const Symbol * ent, const Symbol *type, Literal *initializer, omc::location decl_loc = omc::location())
        : AttributeSymbol(sym, ent, type, decl_loc)
        , initializer(initializer)
    {
    }

    ~SimpleAttributeSymbol()
    {
    }


    /**
     * Gets the initial value for the attribute.
     *
     * @return The initial value as a string.
     */

    string initialization_value(bool type_default) const;

	void post_parse(int pass);

	CodeBlock cxx_declaration_entity();

    // members


    /**
     * The initializer.
     *
     *  C++ literal to initialize simple attribute at entity creation.
     */

    Literal *initializer;

};

