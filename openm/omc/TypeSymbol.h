/**
* @file    TypeSymbol.h
* Declarations for the TypeSymbol class.
*/
// Copyright (c) 2013 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once
#include "Symbol.h"

/**
* TypeSymbol.
*
* The symbol table is initialized with one TypedefTypeSymbol
* corresponding to each kind of type declaration statement in openM++, e.g. time_type, real_type.
* The symbol type (first argument of constructor) is the token associated with the corresponding type name,
* e.g. TK_Time for the time_type statement, TK_index to the index_type statement.
* The one exception is the model_type statement which has an associated TypedefTypeSymbol with 'type' TK_model_type.
* These default symbols are morphed if the corresponding statement is found
* in the model source code.
*/
class TypeSymbol : public Symbol
{
private:
    typedef Symbol super;

public:

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

    void post_parse(int pass);


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
