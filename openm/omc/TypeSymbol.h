/**
* @file    TypeSymbol.h
* Declarations for the TypeSymbol class.
*/
// Copyright (c) 2013-2014 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once
#include "Symbol.h"

/**
* TypeSymbol.
*
*/
class TypeSymbol : public Symbol
{
private:
    typedef Symbol super;

public:
    bool is_base_symbol() const { return false; }

    /**
     * Constructor.
     *
     * @param unm The unique name of the symbol, e.g. "int"
     *
     */

    TypeSymbol(const string unm)
        : Symbol(unm)
    {
        type_id = next_type_id;
        next_type_id++;
    }

    /**
    * Constructor.
    *
    * @param [in,out]  sym The symbol to be morphed.
    */
    TypeSymbol(Symbol *sym, yy::location decl_loc = yy::location())
        : Symbol(sym, decl_loc)
    {
        type_id = next_type_id;
        next_type_id++;
    }


    /**
     * Gets the default initial value for a quantity of this type
     *
     * @return The default initial value as a string
     */

    virtual const string default_initial_value() const = 0;


    /**
     * Identifier for the type.
     */

    int type_id;


    /**
     * type_id for the next TypeSymbol
     * 
     * A shared counter used to assign sequentially increasing type_id to each TypeSymbol.
     */

    static int next_type_id;
};


namespace openm {

    /**
    * Indicates the kind of the type.
    *
    * Used to communicate with meta-data API through the dicId member
    * of things like TypeDicRow.
    */

    enum kind_of_type {
        simple_type = 0,
        logical_type = 1,
        classification_type = 2,
        range_type = 3,
        partition_type = 4,
        link_type = 5,
    };
}

